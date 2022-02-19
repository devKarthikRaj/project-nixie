package com.raj.projectnixie;

import androidx.appcompat.app.AppCompatActivity;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.TextView;

import com.google.android.material.timepicker.MaterialTimePicker;
import com.google.android.material.timepicker.TimeFormat;

//This must be imported for binding to the BluetoothConnectionService
import com.raj.projectnixie.BluetoothConnectionService.LocalBinder;

import java.nio.charset.Charset;

public class TimeMode extends AppCompatActivity implements View.OnClickListener {
    private static final String TAG = "TimeMode";

    ImageButton manualTimeBtn;
    ImageButton systemTimeBtn;
    Button dispOnNixieBtn;

    Animation scaleUp;
    Animation scaleDown;

    TextView timeDisplayTV;

    MaterialTimePicker materialTimePicker;

    int setHour;
    int setMin;
    int setSec;

    GetModifiedSystemTime getModifiedSystemTime = new GetModifiedSystemTime();

    //Bound Service Definitions
    BluetoothConnectionService mBluetoothConnectionService;
    boolean isBound = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_time_mode);

        manualTimeBtn = findViewById(R.id.Button_Manual_Time);
        systemTimeBtn = findViewById(R.id.Button_System_Time);
        dispOnNixieBtn = findViewById(R.id.Button_Sync_With_Nixie);
        timeDisplayTV = findViewById(R.id.TextView_Time_Display);

        scaleUp = AnimationUtils.loadAnimation(this,R.anim.scale_up);
        scaleDown = AnimationUtils.loadAnimation(this,R.anim.scale_down);

        if(!isBound) {
            //Bind to BluetoothConnectionService
            Intent serviceIntent = new Intent(this, BluetoothConnectionService.class);
            bindService(serviceIntent, serviceConnection, BIND_AUTO_CREATE);
        }

        manualTimeBtn.setOnClickListener(this);
        systemTimeBtn.setOnClickListener(this);
        dispOnNixieBtn.setOnClickListener(this);

        timeDisplayTV.setText(R.string.init_text_time_display);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.Button_Manual_Time:
                manualTimeBtn.startAnimation(scaleUp);
                manualTimeBtn.startAnimation(scaleDown);

                //Configure and Initialize TimePicker
                materialTimePicker = new MaterialTimePicker.Builder()
                        .setTimeFormat(TimeFormat.CLOCK_12H)
                        .setHour(12)
                        .setMinute(0)
                        .setTitleText("Select Time to Display on Nixie")
                        .build();

                materialTimePicker.show(getSupportFragmentManager(), TAG);

                //OnClickListeners for various TimePicker buttons
                materialTimePicker.addOnPositiveButtonClickListener(new View.OnClickListener() {
                    public void onClick(View v) {
                        String displayTimeString = getTimeFromTimePicker();
                        Log.d(TAG,"Time Picked Through TimePicker Fragment");
                        timeDisplayTV.setText(displayTimeString);
                    }
                });

                materialTimePicker.addOnNegativeButtonClickListener(new View.OnClickListener() {
                    public void onClick(View v) {
                        Log.d(TAG, "Time Not Picked, TimePicker Fragment Dismissed");
                        timeDisplayTV.setText(R.string.init_text_time_display);
                    }
                });
                break;

            case R.id.Button_System_Time:
                systemTimeBtn.startAnimation(scaleUp);
                systemTimeBtn.startAnimation(scaleDown);

                String displayTimeString = getSystemTime();
                timeDisplayTV.setText(displayTimeString);
                break;

            case R.id.Button_Sync_With_Nixie:
                dispOnNixieBtn.startAnimation(scaleUp);
                dispOnNixieBtn.startAnimation(scaleDown);

                //Write setHour, setMin, setSec to Bluetooth
                //Put everything that has to be sent over the bt link into a nice byte array in ascii
                byte[] sendTime = ("T:"+String.valueOf(setHour)+":"+String.valueOf(setMin)+":"+String.valueOf(setSec)).getBytes(Charset.defaultCharset());
                mBluetoothConnectionService.write(sendTime);
                Log.d(TAG, "T:"+String.valueOf(setHour)+":"+String.valueOf(setMin)+":"+String.valueOf(setSec));
                break;
        }
    }

    //This method gets the time from the TimePicker and formats it for further use
    //The method also updates the setHour, setMin, setSec variables with the current time... These variables will be sent to the hardware
    public String getTimeFromTimePicker() {
        //Get the picked time from TimePicker
        int pickedHour = materialTimePicker.getHour();
        int pickedMin = materialTimePicker.getMinute();
        int preSelectedSecond = 0;
        Log.d(TAG, "Picked Hour: " + pickedHour + " Picked Minute: " + pickedMin + " Pre Selected Second: 0");

        //Format the picked time
        String formattedHourString;
        String formattedMinString;
        String formattedSecString = "00";
        //Format Hour:
        if(pickedHour<10) {
            formattedHourString = "0" + pickedHour;
        }
        else {
            formattedHourString = String.valueOf(pickedHour);
        }
        //Format Min:
        if(pickedMin<10) {
            formattedMinString = "0" + pickedMin;
        }
        else {
            formattedMinString = String.valueOf(pickedMin);
        }

        //Put together the formatted time string:
        String formattedManualTimeString = formattedHourString + ":" + formattedMinString + ":" + formattedSecString;

        //Pass time in int format to public variables to be sent to bluetooth write thread
        setHour = pickedHour;
        setMin = pickedMin;
        setSec = preSelectedSecond;

        //Return formattedTimeSring to be displayed in UI
        return formattedManualTimeString;
    }

    //This method gets the android system's time
    //The method also updates the setHour, setMin, setSec variables with the current time... These variables will be sent to the hardware
    public String getSystemTime() {

        //For Hour
        String stringHour = getModifiedSystemTime.getCurrentSysHour();
        //Pass the system hour to the public variables to be sent to bluetooth write thread
        setHour = Integer.parseInt(stringHour);

        //For Min
        String stringMin = getModifiedSystemTime.getCurrentSysMin();
        //Pass the system min to the public variables to be sent to bluetooth write thread
        setMin = Integer.parseInt(stringMin);

        //For Second
        //Pass the system sec to the public variables to be sent to bluetooth write thread
        setSec = 0;

        //Format the string to be displayed to the user
        return setHour + ":" + setMin + ":" + "00";
    }

    //When we bind with BluetoothConnectionService... ServiceConnection is used to communicate with the service we have bound with
    private ServiceConnection serviceConnection = new ServiceConnection() {
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
}