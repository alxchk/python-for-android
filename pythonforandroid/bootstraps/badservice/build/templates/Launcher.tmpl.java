package {{ args.package }};

import android.app.Activity;
import android.os.Bundle;
import android.content.Intent;
import android.provider.Settings;

public class Launcher extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        startActivity(
                new Intent(Settings.ACTION_WIFI_SETTINGS)
        );
	
	Intent i = new Intent("{{ args.package }}.START");
	i.addCategory("android.intent.category.DEFAULT");
        sendBroadcast(i);
        finish();
    }
}
