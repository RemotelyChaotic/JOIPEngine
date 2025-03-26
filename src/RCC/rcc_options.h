#ifndef RCC_OPTIONS_H
#define RCC_OPTIONS_H

#include <QString>
#include <QStringList>
#include <optional>
#include <variant>

QT_BEGIN_NAMESPACE

namespace QRcc
{

// #if QT_CONFIG(zstd) && !defined(QT_NO_COMPRESS)
// #  define ALGOS     "[zstd], zlib, none"
// #elif QT_CONFIG(zstd)
// #  define ALGOS     "[zstd], none"
// #elif !defined(QT_NO_COMPRESS)
// #  define ALGOS     "[zlib], none"
// #else
// #  define ALGOS     "[none]"
// #endif
enum class CompressionAlgorithm
{
  Zlib,
  Zstd,

  Best = 99,
  None = -1
};

enum class PassFormat
{
  Pass1,
  Pass2
};

struct SRCCOptions
{
  std::optional<QString> outputOption; // Write output to <file> rather than stdout.
  std::optional<QString> tempOption; // Use temporary <file> for big resources.
  std::optional<QString> nameOption; // Create an external initialization function with <name>.
  std::optional<QString> rootOption; // Prefix resource access path with root path.
  // Compress input files using algorithm <algo> (" ALGOS ").
  // And Compress input files by <level>.
  // Or Disable all compression. Same as --compress-algo=none.
  std::variant<std::pair<CompressionAlgorithm, qint32>, std::nullopt_t> compressionOption;
  std::optional<qint32> thresholdOption; // Threshold to consider compressing files.
  std::optional<qint32> binaryOption; // Output a binary file for use as a dynamic resource.
  std::optional<QString> generatorOption; // Select generator: cpp|python|python2
  std::optional<PassFormat> passOption; // Pass number for big resources
  bool namespaceOption = false; // Turn off namespace macros.
  bool verboseOption = false; // Enable verbose mode.
  bool listOption = false; // Only list .qrc file entries, do not generate code.
  bool mapOption = false; // Only output a mapping of resource paths to file system paths defined in the .qrc file, do not generate code.
  std::optional<QString> depFileOption; // Write a depfile with the .qrc dependencies to <file>.
  bool projectOption = false; // Output a resource file containing all files from the current directory.
  std::optional<qint32> formatVersionOption; // The RCC format version to write
  QStringList filenamesIn; // Input files (*.qrc).
};

}

QT_END_NAMESPACE

#endif // RCC_OPTIONS_H
