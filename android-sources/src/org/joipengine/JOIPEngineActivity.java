package org.joipengine;

import org.qtproject.qt5.android.bindings.QtActivity;

import android.os.*;
import android.content.*;
import android.app.*;

import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;

import android.graphics.Color;

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
      if (!checkPermission())
      {
        requestPermission();
      }
    }
    setCustomStatusAndNavBar();
  }

  protected boolean checkPermission()
  {
    return true;
    //int result = ContextCompat.checkSelfPermission(this, android.Manifest.permission.WRITE_EXTERNAL_STORAGE);
    //if (result == PackageManager.PERMISSION_GRANTED)
    //    return true;
    //return false;
  }
  protected void requestPermission()
  {
    return;
    //if (ActivityCompat.shouldShowRequestPermissionRationale(this, android.Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
    //    Toast.makeText(this, "Please allow Write External Storage permission to play your local videos", Toast.LENGTH_LONG).show();
    //} else {
    //    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
    //        requestPermissions(new String[]{android.Manifest.permission.WRITE_EXTERNAL_STORAGE}, 100);
    //  }
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

      //The Window flag 'FLAG_TRANSLUCENT_NAVIGATION' will allow us to paint the background of the navigation bar ourself
      //But we will also have to deal with orientation and OEM specifications, as the nav bar may or may not depend on the orientation of the device
      //window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS | WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION);
      window.getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);

      //Set Statusbar Transparent
      window.setStatusBarColor(Color.TRANSPARENT);
      //Statusbar background is now transparent, but the icons and text are probably white and not really readable, as we have a bright background color
      //We set/force a light theme for the status bar to make those dark
      View decor = window.getDecorView();
      decor.setSystemUiVisibility(decor.getSystemUiVisibility() | View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
    }
  }
}
