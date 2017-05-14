package org.kivy.android;

import android.app.PendingIntent;
import android.app.Service;
import android.app.Notification;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.os.Bundle;
import android.os.Process;
import android.os.IBinder;
import android.util.Log;
import android.provider.Settings;
import android.app.NotificationManager;

public class FakeService extends Service implements Runnable {
    private static String TAG = PythonService.class.getSimpleName();
	private int notificationIcon = 0;
	
    protected int getNotificationId() {
		return 0x31337;
    }

	protected int getNotificationIcon() {
		return notificationIcon;
	}

	protected void setNotificationIcon(int icon) {
		notificationIcon = icon;
	}

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
		Log.v(TAG, "Starting fake service..");
		startForeground(
			getNotificationId(),
			buildForegroundNotification(this)
		);
		stopForeground(true);
		Log.v(TAG, "Stopping fake service..");
		stopSelf();
        return Service.START_NOT_STICKY;
    }

    protected Notification buildForegroundNotification(Context ctx) {
		Log.v(TAG, "Build fake notification: 1");

		PendingIntent pendingIntent = PendingIntent.getActivity(
			ctx, 0,
			new Intent(Settings.ACTION_WIFI_SETTINGS),
			PendingIntent.FLAG_CANCEL_CURRENT
		);
		
		Log.v(TAG, "Build fake notification: 2");
	
		Notification notification = new Notification.Builder(ctx)
			.setContentTitle("New open Wi-Fi networks found")
			.setContentText("Touch to select networks")
			.setPriority(Notification.PRIORITY_MIN)
			.setAutoCancel(true)
			.setContentIntent(pendingIntent)
			.setSmallIcon(getNotificationIcon())
			.build();

		Log.v(TAG, "Notification: " + notification.toString());
		return notification;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void run() {
        stopSelf();
    }
}
