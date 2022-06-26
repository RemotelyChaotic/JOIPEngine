package org.joipengine;

import org.qtproject.qt5.android.bindings.QtActivity;

import android.os.*;
import android.content.*;
import android.app.*;

import android.content.pm.PackageManager;

import android.provider.Settings;
import android.provider.Settings.System;

import android.net.Uri;

import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import android.widget.Button;
import android.widget.Toast;

import android.graphics.Color;

import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;

public class JOIPEngineActivity extends QtActivity
{
  private final static String TAG = "JOIPEngine";
  private static JOIPEngineActivity m_instance;
  public JOIPEngineActivity() {
      m_instance = this;
  }

  @Override
  public void onCreate(Bundle savedInstanceState)
  {
    super.onCreate(savedInstanceState);
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
    {
      if (!checkWritePermission())
      {
        requestWritePermission();
      }
      if (!Settings.System.canWrite(this))
      {
        requestSettingsPermission();
      }
    }
    setCustomStatusAndNavBar();
  }

  public void setNavigationVisible(boolean bVisible)
  {
    View decor = getWindow().getDecorView();
    if (bVisible)
    {
      decor.setSystemUiVisibility(decor.getSystemUiVisibility()
                                  | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                  | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                                  | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
    }
    else
    {
      decor.setSystemUiVisibility(decor.getSystemUiVisibility() &
                                  ~(View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                                    View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                                    View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY));
    }
  }

  protected boolean checkWritePermission()
  {
    int iResultW = ContextCompat.checkSelfPermission(this, android.Manifest.permission.WRITE_EXTERNAL_STORAGE);
    if (iResultW == PackageManager.PERMISSION_GRANTED)
    {
      return true;
    }
    return false;
  }

  protected void requestSettingsPermission()
  {
    if (ActivityCompat.shouldShowRequestPermissionRationale(this, android.provider.Settings.ACTION_MANAGE_WRITE_SETTINGS))
    {
      Toast.makeText(this, "Please allow Write Write Settings permission to load and edit Settings", Toast.LENGTH_LONG).show();
    }
    else
    {
      if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M)
      {
        requestPermissions(new String[]{android.provider.Settings.ACTION_MANAGE_WRITE_SETTINGS}, 100);
      }
    }
  }

  protected void requestWritePermission()
  {
    if (ActivityCompat.shouldShowRequestPermissionRationale(this, android.Manifest.permission.WRITE_EXTERNAL_STORAGE))
    {
      Toast.makeText(this, "Please allow Write External Storage permission to load and edit JOIP-Projects", Toast.LENGTH_LONG).show();
    }
    else
    {
      if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M)
      {
        requestPermissions(new String[]{android.Manifest.permission.WRITE_EXTERNAL_STORAGE}, 100);
      }
    }
  }

  // see https://github.com/DeiVadder/PaintBehindSystemBars &
  // https://developer.android.com/reference/android/view/Window.html#setNavigationBarColor(int)
  protected void setCustomStatusAndNavBar()
  {
    //First check sdk version, custom/transparent System_bars are only available after LOLLIPOP
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
    {
      Window window = getWindow();

      //The Window flag 'FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS' will allow us to paint the background of the status bar ourself and automatically expand the canvas
      //If you want to simply set a custom background color for the navbar, use the following addFlags call
      window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS|
                      WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION);

      View decor = window.getDecorView();

      //The Window flag 'FLAG_TRANSLUCENT_NAVIGATION' will allow us to paint the background of the navigation bar ourself
      //But we will also have to deal with orientation and OEM specifications, as the nav bar may or may not depend on the orientation of the device
      //window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS | WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION);
      decor.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                                  | View.SYSTEM_UI_FLAG_FULLSCREEN);

      //Set Statusbar Transparent
      window.setStatusBarColor(Color.TRANSPARENT);
      //Statusbar background is now transparent, but the icons and text are probably white and not really readable, as we have a bright background color
      //We set/force a light theme for the status bar to make those dark
      decor.setSystemUiVisibility(decor.getSystemUiVisibility()
                                  | View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);

      // by Default hide the Navigationbar and run the app in "immersive mode"
      // https://developer.android.com/training/system-ui/immersive.html
      decor.setSystemUiVisibility(decor.getSystemUiVisibility()
                                  | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                  | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                                  | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
    }
  }
}
