/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Copyright (C) 2018 Intel Corporation.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
** 06.09:2021: Adapted for the JOIP-Engine to handle PhysFs Files by RemotelyChaotic
****************************************************************************/

#include <rcc_runner.h>
#include <rcc.h>

#include <qdebug.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qhashfunctions.h>
#include <qtextstream.h>
#include <qatomic.h>
#include <qglobal.h>
#include <qcoreapplication.h>
#include <qcommandlineoption.h>
#include <qcommandlineparser.h>

#ifdef Q_OS_WIN
#  include <fcntl.h>
#  include <io.h>
#  include <stdio.h>
#endif // Q_OS_WIN

QT_BEGIN_NAMESPACE

namespace QRcc
{

void dumpRecursive(const QDir &dir, QTextStream &out)
{
    const QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot
                                                    | QDir::NoSymLinks);
    for (const QFileInfo &entry : entries) {
        if (entry.isDir()) {
            dumpRecursive(entry.filePath(), out);
        } else {
            out << QLatin1String("<file>")
                << entry.filePath()
                << QLatin1String("</file>\n");
        }
    }
}

int createProject(const QString &outFileName)
{
    QDir currentDir = QDir::current();
    QString currentDirName = currentDir.dirName();
    if (currentDirName.isEmpty())
        currentDirName = QLatin1String("root");

    QFile file;
    bool isOk = false;
    if (outFileName.isEmpty()) {
        isOk = file.open(stdout, QFile::WriteOnly | QFile::Text);
    } else {
        file.setFileName(outFileName);
        isOk = file.open(QFile::WriteOnly | QFile::Text);
    }
    if (!isOk) {
        fprintf(stderr, "Unable to open %s: %s\n",
                outFileName.isEmpty() ? qPrintable(outFileName) : "standard output",
                qPrintable(file.errorString()));
        return 1;
    }

    QTextStream out(&file);
    out << QLatin1String("<!DOCTYPE RCC><RCC version=\"1.0\">\n"
                         "<qresource>\n");

    // use "." as dir to get relative file pathes
    dumpRecursive(QDir(QLatin1String(".")), out);

    out << QLatin1String("</qresource>\n"
                         "</RCC>\n");

    return 0;
}

// Escapes a path for use in a Depfile (Makefile syntax)
QString makefileEscape(const QString &filepath)
{
    // Always use forward slashes
    QString result = QDir::cleanPath(filepath);
    // Spaces are escaped with a backslash
    result.replace(QLatin1Char(' '), QLatin1String("\\ "));
    // Pipes are escaped with a backslash
    result.replace(QLatin1Char('|'), QLatin1String("\\|"));
    // Dollars are escaped with a dollar
    result.replace(QLatin1Char('$'), QLatin1String("$$"));

    return result;
}

void writeDepFile(QIODevice &iodev, const QStringList &depsList, const QString &targetName)
{
    QTextStream out(&iodev);
    out << qPrintable(makefileEscape(targetName));
    out << QLatin1Char(':');

    // Write depfile
    for (int i = 0; i < depsList.size(); ++i) {
        out << QLatin1Char(' ');

        out << qPrintable(makefileEscape(depsList.at(i)));
    }

    out << QLatin1Char('\n');
}

int runRcc(SRCCOptions opts)
{
    QString errorMsg;

    quint8 formatVersion = 3;
    if (opts.formatVersionOption.has_value()) {
        bool ok = false;
        formatVersion = opts.formatVersionOption.value();
        if (!ok) {
            errorMsg = QLatin1String("Invalid format version specified");
        } else if (formatVersion < 1 || formatVersion > 3) {
            errorMsg = QLatin1String("Unsupported format version specified");
        }
    }

    RCCResourceLibrary library(formatVersion);
    if (opts.nameOption.has_value())
        library.setInitName(opts.nameOption.value());
    if (opts.rootOption.has_value()) {
        library.setResourceRoot(QDir::cleanPath(opts.rootOption.value()));
        if (library.resourceRoot().isEmpty()
                || library.resourceRoot().at(0) != QLatin1Char('/'))
            errorMsg = QLatin1String("Root must start with a /");
    }

    if (std::holds_alternative<std::pair<CompressionAlgorithm, qint32>>(opts.compressionOption))
        library.setCompressionAlgorithm(std::get<std::pair<CompressionAlgorithm, qint32>>(opts.compressionOption).first);
    if (formatVersion < 3 && library.compressionAlgorithm() == CompressionAlgorithm::Zstd)
        errorMsg = QLatin1String("Zstandard compression requires format version 3 or higher");
    if (std::holds_alternative<std::nullopt_t>(opts.compressionOption))
        library.setCompressionAlgorithm(CompressionAlgorithm::None);
    if (std::holds_alternative<std::pair<CompressionAlgorithm, qint32>>(opts.compressionOption)) {
        int level = library.parseCompressionLevel(library.compressionAlgorithm(), std::get<std::pair<CompressionAlgorithm, qint32>>(opts.compressionOption).second, &errorMsg);
        library.setCompressLevel(level);
    }
    if (opts.thresholdOption.has_value())
        library.setCompressThreshold(opts.thresholdOption.value());
    if (opts.binaryOption.has_value())
        library.setFormat(RCCResourceLibrary::Binary);
    if (opts.generatorOption.has_value()) {
        auto value = opts.generatorOption.value();
        if (value == QLatin1String("cpp"))
            library.setFormat(RCCResourceLibrary::C_Code);
        else if (value == QLatin1String("python"))
            library.setFormat(RCCResourceLibrary::Python3_Code);
        else if (value == QLatin1String("python2"))
            library.setFormat(RCCResourceLibrary::Python2_Code);
        else
            errorMsg = QLatin1String("Invalid generator: ") + value;
    }

    if (opts.passOption.has_value()) {
        if (opts.passOption.value() == PassFormat::Pass1)
            library.setFormat(RCCResourceLibrary::Pass1);
        else if (opts.passOption.value() == PassFormat::Pass2)
            library.setFormat(RCCResourceLibrary::Pass2);
        else
            errorMsg = QLatin1String("Pass number must be 1 or 2");
    }
    if (opts.namespaceOption)
        library.setUseNameSpace(!library.useNameSpace());
    if (opts.verboseOption)
        library.setVerbose(true);

    const bool list = opts.listOption;
    const bool map = opts.mapOption;
    const bool projectRequested = opts.projectOption;
    const QStringList filenamesIn = opts.filenamesIn;

    for (const QString &file : filenamesIn) {
        if (file == QLatin1String("-"))
            continue;
        else if (!QFile::exists(file)) {
            qWarning("RCC: File does not exist '%s'", qPrintable(file));
            return 1;
        }
    }

    QString outFilename = opts.outputOption.value_or(QString());
    QString tempFilename = opts.tempOption.value_or(QString());
    QString depFilename = opts.depFileOption.value_or(QString());

    if (projectRequested) {
        return createProject(outFilename);
    }

    if (filenamesIn.isEmpty())
        errorMsg = QStringLiteral("No input files specified.");

    if (!errorMsg.isEmpty()) {
        fprintf(stderr, "RCC: %s\n", qPrintable(errorMsg));
        return 1;
    }
    QFile errorDevice;
    errorDevice.open(stderr, QIODevice::WriteOnly|QIODevice::Text);

    if (library.verbose())
        errorDevice.write("Qt resource compiler\n");

    library.setInputFiles(filenamesIn);

    if (!library.readFiles(list || map, errorDevice))
        return 1;

    QFile out;

    // open output
    QIODevice::OpenMode mode = QIODevice::NotOpen;
    switch (library.format()) {
        case RCCResourceLibrary::C_Code:
        case RCCResourceLibrary::Pass1:
        case RCCResourceLibrary::Python3_Code:
        case RCCResourceLibrary::Python2_Code:
            mode = QIODevice::WriteOnly | QIODevice::Text;
            break;
        case RCCResourceLibrary::Pass2:
        case RCCResourceLibrary::Binary:
            mode = QIODevice::WriteOnly;
            break;
    }


    if (outFilename.isEmpty() || outFilename == QLatin1String("-")) {
#ifdef Q_OS_WIN
        // Make sure fwrite to stdout doesn't do LF->CRLF
        if (library.format() == RCCResourceLibrary::Binary)
            _setmode(_fileno(stdout), _O_BINARY);
        // Make sure QIODevice does not do LF->CRLF,
        // otherwise we'll end up in CRCRLF instead of
        // CRLF.
        if (list)
            mode &= ~QIODevice::Text;
#endif // Q_OS_WIN
        // using this overload close() only flushes.
        out.open(stdout, mode);
    } else {
        out.setFileName(outFilename);
        if (!out.open(mode)) {
            const QString msg = QString::fromLatin1("Unable to open %1 for writing: %2\n")
                                .arg(outFilename, out.errorString());
            errorDevice.write(msg.toUtf8());
            return 1;
        }
    }

    // do the task
    if (list) {
        const QStringList data = library.dataFiles();
        for (int i = 0; i < data.size(); ++i) {
            out.write(qPrintable(QDir::cleanPath(data.at(i))));
            out.write("\n");
        }
        return 0;
    }

    if (map) {
        const RCCResourceLibrary::ResourceDataFileMap data = library.resourceDataFileMap();
        for (auto it = data.begin(), end = data.end(); it != end; ++it) {
            out.write(qPrintable(it.key()));
            out.write("\t");
            out.write(qPrintable(QDir::cleanPath(it.value())));
            out.write("\n");
        }
        return 0;
    }

    // Write depfile
    if (!depFilename.isEmpty()) {
        QFile depout;
        depout.setFileName(depFilename);

        if (outFilename.isEmpty() || outFilename == QLatin1String("-")) {
            const QString msg = QString::fromUtf8("Unable to write depfile when outputting to stdout!\n");
            errorDevice.write(msg.toUtf8());
            return 1;
        }

        if (!depout.open(QIODevice::WriteOnly | QIODevice::Text)) {
            const QString msg = QString::fromUtf8("Unable to open depfile %1 for writing: %2\n")
                    .arg(depout.fileName(), depout.errorString());
            errorDevice.write(msg.toUtf8());
            return 1;
        }

        writeDepFile(depout, library.dataFiles(), outFilename);
        depout.close();
    }

    QFile temp;
    if (!tempFilename.isEmpty()) {
        temp.setFileName(tempFilename);
        if (!temp.open(QIODevice::ReadOnly)) {
            const QString msg = QString::fromUtf8("Unable to open temporary file %1 for reading: %2\n")
                    .arg(outFilename, out.errorString());
            errorDevice.write(msg.toUtf8());
            return 1;
        }
    }
    bool success = library.output(out, temp, errorDevice);
    if (!success) {
        // erase the output file if we failed
        out.remove();
        return 1;
    }
    return 0;
}

}

QT_END_NAMESPACE
