package com.raj.projectnixie;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.CompoundButton;
import android.widget.ImageButton;
import android.view.View.OnClickListener;
import android.widget.Toast;

import com.google.android.material.switchmaterial.SwitchMaterial;

public class Home extends AppCompatActivity implements OnClickListener {
    private static final String TAG = "Home";

    ImageButton timeModeBtn;
    ImageButton countdownModeBtn;
    ImageButton ledControlBtn;
    Animation scaleUp;
    Animation scaleDown;

    //The code in this broadcast receiver will execute when bt has been turned off
    //This broadcast receiver will not be unregistered once once started as it has to work in other activities as well
    private final BroadcastReceiver mBroadcastReceiver1 = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            assert action != null; //This is to prevent action.equals() from resulting in a null pointer exception
            if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                final int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);
                if(state == BluetoothAdapter.STATE_OFF) {
                    Log.d(TAG, "Bt turned off");
                    startActivity(new Intent(Home.this, SplashScreen.class));
                }
            }
        }
    };

    //The code in this broadcast receiver will execute when bt connection with remote device has ended
    //This broadcast receiver will not be unregistered once once started as it has to work in other activities as well
    BroadcastReceiver mBroadcastReceiver2 = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if(BluetoothDevice.ACTION_ACL_DISCONNECTED.equals(action)) {
                Log.d(TAG, "Remote bt device has disconnected");
                startActivity(new Intent(Home.this, SplashScreen.class));
           }
        }
    };



    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);

        //Registering broadcast receivers
        IntentFilter filter1 = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
        registerReceiver(mBroadcastReceiver1, filter1);

        IntentFilter filter2 = new IntentFilter(BluetoothDevice.ACTION_ACL_DISCONNECTED);
        registerReceiver(mBroadcastReceiver2, filter2);

        timeModeBtn = findViewById(R.id.Button_Time_Mode);
        countdownModeBtn = findViewById(R.id.Button_Countdown_Mode);
        ledControlBtn = findViewById(R.id.Button_LED_Control);

        scaleUp = AnimationUtils.loadAnimation(this,R.anim.scale_up);
        scaleDown = AnimationUtils.loadAnimation(this,R.anim.scale_down);

        timeModeBtn.setOnClickListener(this);
        countdownModeBtn.setOnClickListener(this);
        ledControlBtn.setOnClickListener(this);

        //Init shared preferences
        SharedPreferences sp = getSharedPreferences("btVeriEnSp", MODE_PRIVATE);
    }

    @Override
    public void onClick(View v) {
        switch(v.getId()) {
            case R.id.Button_Time_Mode:
                timeModeBtn.startAnimation(scaleUp);
                timeModeBtn.startAnimation(scaleDown);
                Log.d(TAG, "Time Mode Button Clicked");
                startActivity(new Intent(Home.this, TimeMode.class));
                break;
            case R.id.Button_Countdown_Mode:
                countdownModeBtn.startAnimation(scaleUp);
                countdownModeBtn.startAnimation(scaleDown);
                Log.d(TAG, "Countdown Mode Button Clicked");
                startActivity(new Intent(Home.this, CountdownMode.class));
                break;
            case R.id.Button_LED_Control:
                ledControlBtn.startAnimation(scaleUp);
                ledControlBtn.startAnimation(scaleDown);
                Log.d(TAG, "LED Control Button Clicked");
                startActivity(new Intent(Home.this, LedControl.class));
                break;
        }
    }
}