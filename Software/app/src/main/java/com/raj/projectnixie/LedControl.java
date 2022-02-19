package com.raj.projectnixie;

import androidx.annotation.NonNull;
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
import android.widget.RadioButton;
import android.widget.RadioGroup;

import com.google.android.material.slider.Slider;

import java.nio.charset.Charset;

import com.raj.projectnixie.BluetoothConnectionService.LocalBinder;

public class LedControl extends AppCompatActivity implements View.OnClickListener{
    private static final String TAG = "LedControl";

    RadioGroup ledModeRadGrp;
    RadioButton ledMode1RadBtn;
    int ledMode1RadBtnId = 1;
    RadioButton ledMode2RadBtn;
    int ledMode2RadBtnId = 2;
    RadioButton ledMode3RadBtn;
    int ledMode3RadBtnId = 3;
    RadioButton ledMode4RadBtn;
    int ledMode4RadBtnId = 4;
    RadioButton ledMode5RadBtn;
    int ledMode5RadBtnId = 5;
    RadioButton ledMode6RadBtn;
    int ledMode6RadBtnId = 6;
    RadioButton ledMode7RadBtn;
    int ledMode7RadBtnId = 7;
    RadioButton ledMode8RadBtn;
    int ledMode8RadBtnId = 8;

    Slider ledBrightnessSlider;
    Slider ledColorSlider;

    Button btnConfigLeds;

    Animation scaleUp;
    Animation scaleDown;

    int ledModeNum;
    int ledBrightnessNum;
    String ledBrightnessNumString;
    int ledColorHexNumAsDec;
    int rgbR;
    String rgbRString;
    int rgbG;
    String rgbGString;
    int rgbB;
    String rgbBString;

    //Bound Service Definitions
    BluetoothConnectionService mBluetoothConnectionService;
    boolean isBound = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_led_control);
        ledModeRadGrp = findViewById(R.id.RadioGroup_LED_Mode);
        ledMode1RadBtn = findViewById(R.id.RadioButton_LED_Mode1);
        ledMode1RadBtn.setId(ledMode1RadBtnId);
        ledMode1RadBtn.toggle();
        ledMode2RadBtn = findViewById(R.id.RadioButton_LED_Mode2);
        ledMode2RadBtn.setId(ledMode2RadBtnId);
        ledMode3RadBtn = findViewById(R.id.RadioButton_LED_Mode3);
        ledMode3RadBtn.setId(ledMode3RadBtnId);
        ledMode4RadBtn = findViewById(R.id.RadioButton_LED_Mode4);
        ledMode4RadBtn.setId(ledMode4RadBtnId);
        ledMode5RadBtn = findViewById(R.id.RadioButton_LED_Mode5);
        ledMode5RadBtn.setId(ledMode5RadBtnId);
        ledMode6RadBtn = findViewById(R.id.RadioButton_LED_Mode6);
        ledMode6RadBtn.setId(ledMode6RadBtnId);
        ledMode7RadBtn = findViewById(R.id.RadioButton_LED_Mode7);
        ledMode7RadBtn.setId(ledMode7RadBtnId);
        ledMode8RadBtn = findViewById(R.id.RadioButton_LED_Mode8);
        ledMode8RadBtn.setId(ledMode8RadBtnId);
        ledBrightnessSlider = findViewById(R.id.Slider_LED_Brightness);
        ledColorSlider = findViewById(R.id.Slider_LED_Color);
        btnConfigLeds = findViewById(R.id.Button_Config_LEDs);

        scaleUp = AnimationUtils.loadAnimation(this,R.anim.scale_up);
        scaleDown = AnimationUtils.loadAnimation(this,R.anim.scale_down);

        btnConfigLeds.setOnClickListener(this);

        ledModeNum = 1;
        ledBrightnessNum = 50;
        ledColorHexNumAsDec = 0;
        rgbR = 0;
        rgbG = 192;
        rgbB = 240;

        //Bind to BluetoothConnectionService
        Intent serviceIntent = new Intent(this, BluetoothConnectionService.class);
        bindService(serviceIntent, serviceConnection, Context.BIND_AUTO_CREATE);

        //If checked radio button has changed...
        ledModeRadGrp.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener()
        {
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                //Get the LED Mode chosen by user
                int selectedLedModeId = ledModeRadGrp.getCheckedRadioButtonId();
                Log.d(TAG, "Mode Selected: " + String.valueOf(selectedLedModeId));
                switch(selectedLedModeId) {
                    case 1:
                        ledModeNum = 1;
                        break;
                    case 2:
                        ledModeNum = 2;
                        break;
                    case 3:
                        ledModeNum = 3;
                        break;
                    case 4:
                        ledModeNum = 4;
                        break;
                    case 5:
                        ledModeNum = 5;
                        break;
                    case 6:
                        ledModeNum = 6;
                        break;
                    case 7:
                        ledModeNum = 7;
                        break;
                    case 8:
                        ledModeNum = 8;
                        break;
                }
            }
        });

        //If brightness slider value has changed...
        ledBrightnessSlider.addOnChangeListener(new Slider.OnChangeListener() {
            @Override
            public void onValueChange(@NonNull Slider slider, float value, boolean fromUser) {
                ledBrightnessNum = Math.round(value);
                }
        });

        //If color slider value has changed...
        ledColorSlider.addOnChangeListener(new Slider.OnChangeListener() {
            @Override
            public void onValueChange(@NonNull Slider slider, float value, boolean fromUser) {
                ledColorHexNumAsDec = Math.round(value);

                //LED Color Hex to RGB Conversion
                //Convert the hex number to string and store in char array
                char[] hexToRgbArray = String.valueOf(ledColorHexNumAsDec).toCharArray();

                //Splitting the hex digits (first 2 digits go to red... next 2 digits go to green... last 2 digits go to blue)
                rgbR = Integer.parseInt(String.valueOf(hexToRgbArray[0] )+ String.valueOf(hexToRgbArray[1]));
                rgbG = Integer.parseInt(String.valueOf(hexToRgbArray[2] )+ String.valueOf(hexToRgbArray[3]));
                rgbB = Integer.parseInt(String.valueOf(hexToRgbArray[4] )+ String.valueOf(hexToRgbArray[5]));
            }
        });
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.Button_Config_LEDs:
                btnConfigLeds.startAnimation(scaleUp);
                btnConfigLeds.startAnimation(scaleDown);

                if(ledBrightnessNum<100 && ledBrightnessNum!=0) {
                    ledBrightnessNumString = "0" + String.valueOf(ledBrightnessNum);
                }
                else if(ledBrightnessNum == 0) {
                    ledBrightnessNumString = "00" + String.valueOf(ledBrightnessNum);
                }
                else {
                    ledBrightnessNumString = String.valueOf(ledBrightnessNum);
                }

                if(rgbR<100 && rgbR!=0) {
                    rgbRString = "0" + String.valueOf(rgbR);
                }
                else if(rgbR == 0) {
                    rgbRString = "00" + String.valueOf(rgbR);
                }
                else {
                    rgbRString = String.valueOf(rgbR);
                }

                if(rgbG<100 && rgbG!=0) {
                    rgbGString = "0" + String.valueOf(rgbG);
                }
                else if(rgbG == 0) {
                    rgbGString = "00" + String.valueOf(rgbG);
                }
                else {
                    rgbGString = String.valueOf(rgbG);
                }

                if(rgbB<100 && rgbB!=0) {
                    rgbBString = "0" + String.valueOf(rgbB);
                }
                else if(rgbB == 0) {
                    rgbBString = "00" + String.valueOf(rgbB);
                }
                else {
                    rgbBString = String.valueOf(rgbB);
                }

                //Construct LED mode config string to be sent to hardware
                Log.d(TAG, "L:"+String.valueOf(ledModeNum)+":"+ledBrightnessNumString+":"+rgbRString+":"+rgbGString+":"+rgbBString);
                byte[] ledConfig = ("L:"+String.valueOf(ledModeNum)+":"+ledBrightnessNumString+":"+rgbRString+":"+rgbGString+":"+rgbBString).getBytes(Charset.defaultCharset());

                //Send LED mode config to hardware
                mBluetoothConnectionService.write(ledConfig);
                break;
        }
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
