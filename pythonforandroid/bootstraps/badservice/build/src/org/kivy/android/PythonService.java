package org.kivy.android;

import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.os.Bundle;
import android.os.Process;
import android.os.IBinder;
import android.util.Log;
import org.kivy.android.FakeService;

public class PythonService extends FakeService implements Runnable {
    private static String TAG = PythonService.class.getSimpleName();

    /**
     * Intent that started the service
     */
    private Intent startIntent = null;

    private Thread pythonThread = null;

    // Python environment variables
    private String androidPrivate;
    private String androidArgument;
    private String pythonName;
    private String pythonHome;
    private String pythonPath;
    private String serviceEntrypoint;
    private String pythonServiceArgument;
    private int notificationIcon;

    public static PythonService mService = null;

    public int getStartType() {
        return Service.START_STICKY;
    }

    public boolean getStartForeground() {
        return true;
    }

    public boolean getAutoRestart() {
        return true;
    }

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onCreate() {
        Log.v(TAG, "Device: " + android.os.Build.DEVICE);
        Log.v(TAG, "Model: " + android.os.Build.MODEL);
        AssetExtract.extractAsset(getApplicationContext(), "private.mp3", getFilesDir());
        super.onCreate();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (pythonThread != null) {
            Log.v(TAG, "Service exists, do not start again");
            return START_NOT_STICKY;
        }
	
        startIntent = intent;

        Log.v(TAG, "Receive intent at onStartCommand: " + intent.toString());

        Bundle extras = intent.getExtras();

	for (String key : extras.keySet()) {
                Object value = extras.get(key);
                Log.d(TAG, String.format("%s %s (%s)", key,
                value.toString(), value.getClass().getName()));
        }

        androidPrivate = extras.getString("androidPrivate");
        androidArgument = extras.getString("androidArgument");
        serviceEntrypoint = extras.getString("serviceEntrypoint");
        pythonName = extras.getString("pythonName");
        pythonHome = extras.getString("pythonHome");
        pythonPath = extras.getString("pythonPath");
        pythonServiceArgument = extras.getString("pythonServiceArgument");

	setNotificationIcon(extras.getInt("notificationIcon"));

        Log.v(TAG, "Starting Python thread");
        pythonThread = new Thread(this);
        pythonThread.start();

        if (getStartForeground()) {
	    Log.v(TAG, "Foreground requested");
            doStartForeground(extras);
        }

        return getStartType();
    }

    protected void doStartForeground(Bundle extras) {
	Context ctx = getApplicationContext();
	startForeground(getNotificationId(), buildForegroundNotification(this));
	Log.v(TAG, "Request fake service start..");
	// https://code.google.com/p/android/issues/detail?id=213309
	ctx.startService(new Intent(ctx, FakeService.class));
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onDestroy() {
        super.onDestroy();
        pythonThread = null;
        if (getAutoRestart() && startIntent != null) {
            Log.v(TAG, "Service restart requested");
            startService(startIntent);
        }
        Process.killProcess(Process.myPid());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void run() {
        PythonUtil.loadLibraries(getFilesDir());
        mService = this;
        nativeStart(androidPrivate, androidArgument, serviceEntrypoint, pythonName, pythonHome,
                pythonPath, pythonServiceArgument);
        stopSelf();
    }

    /**
     * @param androidPrivate        Directory for private files
     * @param androidArgument       Android path
     * @param serviceEntrypoint     Python file to execute first
     * @param pythonName            Python name
     * @param pythonHome            Python home
     * @param pythonPath            Python path
     * @param pythonServiceArgument Argument to pass to Python code
     */
    public static native void nativeStart(String androidPrivate, String androidArgument,
                                          String serviceEntrypoint, String pythonName,
                                          String pythonHome, String pythonPath,
                                          String pythonServiceArgument);
}
