package com.raj.projectnixie;

import androidx.appcompat.app.AppCompatActivity;

import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import java.nio.charset.Charset;

import com.raj.projectnixie.BluetoothConnectionService.LocalBinder;

public class CountdownMode extends AppCompatActivity implements View.OnClickListener{
    private static final String TAG = "CountdownMode";

    EditText EThour;
    EditText ETmin;
    EditText ETsec;

    Button dispOnNixieBtn;

    Animation scaleUp;
    Animation scaleDown;

    //Bound Service Definitions
    BluetoothConnectionService mBluetoothConnectionService;
    boolean isBound = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_countdown_mode);

        EThour = findViewById(R.id.EditText_Hours);
        ETmin = findViewById(R.id.EditText_Mins);
        ETsec = findViewById(R.id.EditText_Secs);

        dispOnNixieBtn = findViewById(R.id.Button_Sync_With_Nixie);

        scaleUp = AnimationUtils.loadAnimation(this,R.anim.scale_up);
        scaleDown = AnimationUtils.loadAnimation(this,R.anim.scale_down);

        //Bind to BluetoothConnectionService
        Intent serviceIntent = new Intent(this, BluetoothConnectionService.class);
        bindService(serviceIntent, serviceConnection, Context.BIND_AUTO_CREATE);

        dispOnNixieBtn.setOnClickListener(this);

        EThour.setOnClickListener(view -> EThour.setText(""));

        ETmin.setOnClickListener(view -> ETmin.setText(""));

        ETsec.setOnClickListener(view -> ETsec.setText(""));

        IntentFilter BTDisconnectedFromRemoteDeviceIntent = new IntentFilter(BluetoothDevice.ACTION_ACL_DISCONNECTED);
        registerReceiver(BTDisconnectedBroadcastReceiver, BTDisconnectedFromRemoteDeviceIntent);
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.Button_Sync_With_Nixie) {
            dispOnNixieBtn.startAnimation(scaleUp);
            dispOnNixieBtn.startAnimation(scaleDown);

            //If the user has not entered anything...
            if (!EThour.getText().toString().equals("") && !ETmin.getText().toString().equals("") && !ETsec.getText().toString().equals("")) {
                //If the user entry is valid...
                if (Integer.parseInt(EThour.getText().toString()) < 100 && Integer.parseInt(EThour.getText().toString()) >= 0
                        && Integer.parseInt(ETmin.getText().toString()) < 60 && Integer.parseInt(ETmin.getText().toString()) >= 0
                        && Integer.parseInt(ETsec.getText().toString()) < 60 && Integer.parseInt(ETsec.getText().toString()) >= 0) {
                    byte[] sendCountdownTime = ("C:" + EThour.getText().toString() + ":" + ETmin.getText().toString() + ":" + ETsec.getText().toString()).getBytes(Charset.defaultCharset());
                    mBluetoothConnectionService.write(sendCountdownTime);
                    Log.d(TAG, EThour.getText().toString() + ":" + ETmin.getText().toString() + ":" + ETsec.getText().toString());
                }
                //If the user entry is invalid...
                else {
                    Toast.makeText(getApplicationContext(), "Enter a Valid Time to Countdown From !!!", Toast.LENGTH_SHORT).show();
                }
            } else if (EThour.getText().toString().equals("Hours") || !ETmin.getText().toString().equals("Mins")) {
                Toast.makeText(getApplicationContext(), "Fill in all the fields !!!", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(getApplicationContext(), "Enter a Time to Countdown From !!!", Toast.LENGTH_SHORT).show();
            }
        }
    }

    //When we bind with BluetoothConnectionService... ServiceConnection is used to communicate with the service we have bound with
    private final ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        //What happens when we first bind with the service...
        public void onServiceConnected(ComponentName name, IBinder service) {
            LocalBinder mBinder = (LocalBinder) service;
            mBluetoothConnectionService = mBinder.getService();
            //Don't need to pass handler here cuz handler has alr been passed when SplashScreen.java initially bounded to the service!
            isBound = true;
        }

        //What happens when we unbind from the service...
        @Override
        public void onServiceDisconnected(ComponentName name) {
            isBound = false;
        }
    };

    private final BroadcastReceiver BTDisconnectedBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            startActivity(new Intent(CountdownMode.this, SplashScreen.class));
        }
    };
}












