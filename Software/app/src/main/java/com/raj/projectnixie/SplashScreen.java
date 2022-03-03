package com.raj.projectnixie;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.Manifest;
import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Set;
import java.util.UUID;

//This must be imported for binding to the BluetoothConnectionService
import com.raj.projectnixie.BluetoothConnectionService.LocalBinder;

@SuppressLint("CustomSplashScreen")
public class SplashScreen extends AppCompatActivity implements PairedDevicesRVClickInterface {
    private static final String TAG = "SplashScreen";

    TextView tvBTStatusMonitor;

    Button btnBtListPairedDevices;
    Button btnGoToBtSettings;
    Button btnDisconnect;

    Animation scaleUp;
    Animation scaleDown;

    BluetoothAdapter mBluetoothAdapter;
    BluetoothDeviceListAdapter mBluetoothDeviceListAdapter;
    ArrayList<BluetoothDevice> mBTDevicesInfo = new ArrayList<>(); // Data model containing pair Bluetooth devices list (The phone has this data model in it!)
    BluetoothDevice touchedDevice;
    String remoteDeviceName;
    String remoteDeviceAddress;

    RecyclerView rvPairedDevices;
    LinearLayoutManager mLayoutManager;

    // This UUID is specially for use with HC05 Bluetooth Modules (Yet to be tested for other devices)
    private static final UUID MY_HC05_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    //Bound Service Definitions
    BluetoothConnectionService mBluetoothConnectionService;
    boolean isBound = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_splash_screen);

        //Init shared preferences
        SharedPreferences sp = getSharedPreferences("btVeriEnSp", MODE_PRIVATE);
        //If disable bluetooth verification toggle switch is checked...
        if(sp.getBoolean("btVeriEn", false)) {
            startActivity(new Intent(SplashScreen.this, Home.class));
        }

        tvBTStatusMonitor = findViewById(R.id.TextView_Bluetooth_Status_Monitor);

        btnBtListPairedDevices = findViewById(R.id.Button_List_Paired_Devices);
        btnGoToBtSettings = findViewById(R.id.Button_GoTo_Bluetooth_Settings);
        btnDisconnect = findViewById(R.id.Button_Bluetooth_Disconnect);

        scaleUp = AnimationUtils.loadAnimation(this,R.anim.scale_up);
        scaleDown = AnimationUtils.loadAnimation(this,R.anim.scale_down);

        //Bind to BluetoothConnectionService
        Intent serviceIntent = new Intent(this, BluetoothConnectionService.class);
        startService(serviceIntent); //Only need to start service once... put this in the activity where you start the service
        bindService(serviceIntent, serviceConnection, Context.BIND_AUTO_CREATE); //this has to be done in all activities that bind to the service

        // Initialize recycler view adapter and layout manager
        //-
        // To resolve no adapter attached skipping layout runtime error
        mBluetoothDeviceListAdapter = new BluetoothDeviceListAdapter(mBTDevicesInfo, this);
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        mBTDevicesInfo = new ArrayList<>();
        enableBt();

        rvPairedDevices = findViewById(R.id.RecyclerView_Paired_Bluetooth_Devices);
        rvPairedDevices.setHasFixedSize(false);
        mLayoutManager = new LinearLayoutManager(this);
        mLayoutManager.setOrientation(LinearLayoutManager.VERTICAL);
        rvPairedDevices.setLayoutManager(mLayoutManager);
        rvPairedDevices.setAdapter(mBluetoothDeviceListAdapter);

        // Intent to detect Bluetooth Connection Established with remote device (Note: Intent Filters work hand in hand with broadcast receivers)
        // ACTION_ACL_CONNECTED is used instead of BLUETOOTH_CONNECTION_STATE_CHANGED cuz the UUID used is weird (HC05 problems!!!)
        IntentFilter BTConnectedToRemoteDeviceIntent = new IntentFilter(BluetoothDevice.ACTION_ACL_CONNECTED);
        registerReceiver(BTConnectedBroadcastReceiver, BTConnectedToRemoteDeviceIntent);

        IntentFilter BTDisconnectedFromRemoteDeviceIntent = new IntentFilter(BluetoothDevice.ACTION_ACL_DISCONNECTED);
        registerReceiver(BTDisconnectedBroadcastReceiver, BTDisconnectedFromRemoteDeviceIntent);

        // OnClickListeners
        btnBtListPairedDevices.setOnClickListener(view -> {
            btnBtListPairedDevices.startAnimation(scaleUp);
            btnBtListPairedDevices.startAnimation(scaleDown);

            if (mBluetoothAdapter.isEnabled()) {
                Log.d(TAG, "onClick: Listing paired devices (if any)");
                ListPairedBtDevices();
            } else {
                Toast.makeText(getBaseContext(), "Enable Bluetooth to view paired devices", Toast.LENGTH_SHORT).show();
                enableBt();
            }
        });

        btnGoToBtSettings.setOnClickListener(view -> {
            btnGoToBtSettings.startAnimation(scaleUp);
            btnGoToBtSettings.startAnimation(scaleDown);
            startActivityForResult(new Intent(android.provider.Settings.ACTION_BLUETOOTH_SETTINGS), 0);
        });

        btnDisconnect.setOnClickListener(view -> {
            btnDisconnect.startAnimation(scaleUp);
            btnDisconnect.startAnimation(scaleDown);
            disconnect();
        });
    }

    // Turn on the device's Bluetooth
    private void enableBt() {
        if(!mBluetoothAdapter.isEnabled()) {
            Log.d(TAG, "toggleBt: Request to enable Bluetooth");
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            if(ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH) == PackageManager.PERMISSION_GRANTED)
                startActivity(enableBtIntent);
        }
    }

    // Lists all devices that the device has "already paired with"
    private void ListPairedBtDevices() {
        Set<BluetoothDevice> pairedBTDevices = null;
        if(ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH) == PackageManager.PERMISSION_GRANTED)
            pairedBTDevices = mBluetoothAdapter.getBondedDevices();

        assert pairedBTDevices != null;
        if(pairedBTDevices.size() == 0) {
            Toast.makeText(this, "No Paired Devices", Toast.LENGTH_SHORT).show();
        }
        else {
            mBTDevicesInfo.clear();
            // The ":" operator usage in for loop...
            // For every item in pairedBTDevices, equal that particular item to individualBTDevices and go through the for loop, then repeat!
            for(BluetoothDevice individualBTDeviceInfo :  pairedBTDevices) {
                mBTDevicesInfo.add(individualBTDeviceInfo);
                Log.d(TAG, individualBTDeviceInfo.getName() + "\n" + individualBTDeviceInfo.getAddress());
            }
        }

        // To display paired devices details in recycler view
        mBluetoothDeviceListAdapter = new BluetoothDeviceListAdapter(mBTDevicesInfo, this);
        rvPairedDevices.setLayoutManager(mLayoutManager);
        rvPairedDevices.setAdapter(mBluetoothDeviceListAdapter);
    }

    // Recycler View onItemClick method
    @Override
    public void onItemClick(int position) {
        // Cancel discovery before trying to make the connection because discovery will slow down a connection
        if(ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH) == PackageManager.PERMISSION_GRANTED)
            mBluetoothAdapter.cancelDiscovery();

        touchedDevice = mBTDevicesInfo.get(position);

        remoteDeviceName = touchedDevice.getName();
        remoteDeviceAddress = touchedDevice.getAddress();

        //connect to bluetooth bond
        mBTDevicesInfo.get(position).createBond();

        startConnection();
    }

    // Recycler View onItemLongClick method
    @Override
    public void onLongItemClick(int position) {
        //do nothing
    }

    // ***Remember the connection will fail and app will crash if you attempt to connect to an unpaired device***
    private void startConnection() {
        mBluetoothConnectionService.startClient(touchedDevice, MY_HC05_UUID);
    }

    private void disconnect() {
        mBluetoothConnectionService.cancelConnectedThread();
    }

    // Broadcast receivers to monitor Bluetooth connection status and update tvBTStatusMonitor
    //This code will execute when the bt connected broadcast is received
    private final BroadcastReceiver BTConnectedBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Resources resources = getResources();
            tvBTStatusMonitor.setText(R.string.connected_to_text);
            tvBTStatusMonitor.append(" ");
            tvBTStatusMonitor.append(String.format(resources.getString(R.string.remote_device_name_text), remoteDeviceName));
        }
    };

    //This code will execute when the bt disconnected broadcast is received
    private final BroadcastReceiver BTDisconnectedBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            tvBTStatusMonitor.setText(R.string.disconnected_text);
        }
    };

    //The code in this handler will execute when an incoming message is detected
    @SuppressLint("HandlerLeak")
    //Looper.getMainLooper()).postDelayed(new Runnable() {
    public final Handler handler = new Handler() {
        public void handleMessage(Message msg) {
            Bundle recBundle = msg.getData();
            String incomingMessage = recBundle.getString("incomingMessage");
            Log.d(TAG,"HandlerMsg: " + incomingMessage);

            //Send to incomingMessage string into a char array for further processing
            assert incomingMessage != null; //Make sure incomingMessage isn't null... Will cause a null pointer exception if its null
            char[] ch = new char[incomingMessage.length()];
            for (int i = 0; i < incomingMessage.length(); i++) {
                ch[i] = incomingMessage.charAt(i);
            }

            //Level 1 Check (Refer to software documentation for security verification procedure)
            if(ch[0]=='R' && ch[1]=='E' && ch[2]=='Q' && ch[3]=='_' && ch[4]=='C' && ch[5]=='H' && ch[6]=='E' && ch[7]=='C' && ch[8]=='K' && ch[9]=='_' && ch[10]=='1') {
                mBluetoothConnectionService.write(("REQ_CONN").getBytes(Charset.defaultCharset()));
            }

            //Level 1 Check Pass
            if(ch[0]=='C' && ch[1]=='O' && ch[2]=='N' && ch[3]=='N' && ch[4]=='_' && ch[5]=='P' && ch[6]=='A' && ch[7]=='S' && ch[8]=='S') {
                Toast.makeText(getApplicationContext(),"Successfully connected to Nixie Clock!",Toast.LENGTH_SHORT).show();
                //Go to home activity
                startActivity(new Intent(getApplicationContext(), Home.class));
            }

            //Level 1 Check Fail
            if(ch[0]=='C' && ch[1]=='O' && ch[2]=='N' && ch[3]=='N' && ch[4]=='_' && ch[5]=='F' && ch[6]=='A' && ch[7]=='I' && ch[8]=='L') {
                mBluetoothConnectionService.write(("REQ_CONN").getBytes(Charset.defaultCharset()));
                Toast.makeText(getApplicationContext(),"Re-connecting with hardware!",Toast.LENGTH_SHORT).show();
            }
        }
    };

    //When we bind with BluetoothConnectionService... ServiceConnection is used to communicate with the service we have bound with
    private final ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        //What happens when we first bind with the service...
        public void onServiceConnected(ComponentName name, IBinder service) {
            LocalBinder mBinder = (LocalBinder) service;
            mBluetoothConnectionService = mBinder.getService();

            //Can't pass the handler through the constructor of the service class (no point having a constructor in a service class so
            //we gotta pass it through a method that I coded into the service (something like a setter method)
            mBluetoothConnectionService.setHandler(handler);

            isBound = true;
        }

        //What happens when we unbind from the service...
        @Override
        public void onServiceDisconnected(ComponentName name) { isBound = false; }
    };
}
