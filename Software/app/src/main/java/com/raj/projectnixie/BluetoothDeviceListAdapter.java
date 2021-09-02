package com.raj.projectnixie;

import android.bluetooth.BluetoothDevice;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;

public class BluetoothDeviceListAdapter extends RecyclerView.Adapter<BluetoothDeviceListAdapter.MyViewHolder> {
    private ArrayList<BluetoothDevice> mBTDevicesInfo;
    private PairedDevicesRVClickInterface mPairedDevicesRVClickInterface;

    // Constructor
    public BluetoothDeviceListAdapter(ArrayList<BluetoothDevice> btDevicesInfo, PairedDevicesRVClickInterface pairedDevicesRVClickInterface) {
        this.mBTDevicesInfo = btDevicesInfo;
        this.mPairedDevicesRVClickInterface = pairedDevicesRVClickInterface;
    }

    public class MyViewHolder extends RecyclerView.ViewHolder {
        TextView mBTName;
        TextView mBTAddr;

        /*
            A ViewHolder is more than just a dumb object that only holds the item’s views. It is the very object that represents each item in
            our collection and will be used to display it. Each item in a recycler view has its own view holder
        */
        public MyViewHolder(@NonNull View itemView) {
            super(itemView);
            mBTName = itemView.findViewById(R.id.bt_name);
            mBTAddr = itemView.findViewById(R.id.bt_mac_addr);

            // OnClickListeners for each and every way in which the user is expected to "touch" the recycler view
            itemView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    mPairedDevicesRVClickInterface.onItemClick(getLayoutPosition());
                }
            });

            itemView.setOnLongClickListener(new View.OnLongClickListener() {
                @Override
                public boolean onLongClick(View view) {
                    mPairedDevicesRVClickInterface.onLongItemClick(getLayoutPosition());

                    return true;
                }
            });
            //---------------------------------------------------------------------------------------------------
        }
    }

    /*
        onCreateViewHolder is called when recycler view needs a new ViewHolder to represent an item
        When ever a new item previously not visible, but just became visible cuz the user scrolled up/down - then the onCreateViewHolder
        method is called to display that item
        In short, whenever the recycler view needs to (display a new item / item that just became visible), onCreateViewHolder is called
    */
    @Override
    public BluetoothDeviceListAdapter.MyViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        // Inflate the recycler view item layout file which is in the main xml
        View itemView =  LayoutInflater.from(parent.getContext()).inflate(R.layout.rv_item_bt_paired_devices, parent, false);

        // Create an instance of MyViewHolder and pass itemView to it
        // itemView contains the inflated recyclerview_item layout file
        MyViewHolder vh = new MyViewHolder(itemView);
        return vh;
    }

    @Override
    public void onBindViewHolder(MyViewHolder holder, int position) {
        // Get the data of the "current item to be displayed" from the data model instance created earlier
        // and stores it in the "DataMode;" data type
        BluetoothDevice currentItem = mBTDevicesInfo.get(position);

        // Get the respective info from the data model class and display it in the recyclerview_item xml which is in in the main xml
        holder.mBTName.setText(currentItem.getName());
        holder.mBTAddr.setText(currentItem.getAddress());
    }

    @Override
    public int getItemCount() {
        return mBTDevicesInfo.size();
    }
}
