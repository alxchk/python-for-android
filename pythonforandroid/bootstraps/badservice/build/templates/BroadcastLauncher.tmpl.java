package {{ args.package }};

import {{ args.package }}.R;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.Context;
import android.util.Log;
import android.widget.Toast;
import org.kivy.android.PythonService;

public class BroadcastLauncher extends BroadcastReceiver {

    @Override
    public void onReceive(Context ctx, Intent i) {
        Toast.makeText(ctx, "Hello from pupy!", Toast.LENGTH_LONG).show();
    
        String path = ctx.getFilesDir().getAbsolutePath();
        Intent intent = new Intent(ctx, PythonService.class);

        Log.v("{{ args.name|capitalize }}",
	      "Starting service {{ args.name|capitalize }}:{{ entrypoint }}, dir: " + path);

        intent.putExtra("androidPrivate", path);
        intent.putExtra("androidArgument", path);
        intent.putExtra("serviceEntrypoint", "main.pyo");
        intent.putExtra("serviceTitle", "{{ args.name|capitalize }}");
        intent.putExtra("serviceDescription", "");
        intent.putExtra("pythonName", "{{ args.name }}");
        intent.putExtra("pythonHome", path);
        intent.putExtra("pythonPath", path + ":" + path + "/lib");
        intent.putExtra("pythonServiceArgument", "{{ args.package }}");
	intent.putExtra("notificationIcon", R.drawable.stat_notify_wifi_in_range);
        ctx.startService(intent);
    }
}
