package org.joipengine;

import org.qtproject.example.JOIPEngine.R;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;

import android.content.Context;
import android.content.Intent;

import android.graphics.Color;
import android.graphics.BitmapFactory;

import android.app.NotificationChannel;

public class JOIPNotificationClient
{
  private static NotificationManager m_notificationManager;
  private static Notification.Builder m_builder;

  public JOIPNotificationClient() {}

  public static void notify(Context context, String title, String message)
  {
    try
    {
      m_notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);

      if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O)
      {
        int importance = NotificationManager.IMPORTANCE_DEFAULT;
        NotificationChannel notificationChannel = new NotificationChannel("JOIPEngine", "JOIPEngine", importance);
        m_notificationManager.createNotificationChannel(notificationChannel);
        m_builder = new Notification.Builder(context, notificationChannel.getId());
      }
      else
      {
        m_builder = new Notification.Builder(context);
      }

      m_builder.setSmallIcon(R.drawable.icon)
              .setLargeIcon(BitmapFactory.decodeResource(context.getResources(), R.drawable.icon))
              .setContentTitle(title)
              .setContentText(message)
              .setDefaults(Notification.DEFAULT_SOUND)
              .setColor(0xffaa00aa)
              .setAutoCancel(true);

      m_notificationManager.notify(0, m_builder.build());
    }
    catch (Exception e)
    {
      e.printStackTrace();
    }
  }
}
