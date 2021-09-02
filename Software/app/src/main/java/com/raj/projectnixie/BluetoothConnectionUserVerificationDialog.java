package com.raj.projectnixie;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;

import androidx.appcompat.app.AppCompatDialogFragment;

public class BluetoothConnectionUserVerificationDialog extends AppCompatDialogFragment {
    private EditText etVerificationCode;
    private PopUpDialogListener popUpDialogListener;

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity(), R.style.MyDialogTheme);
        LayoutInflater inflater = getActivity().getLayoutInflater();
        View view = inflater.inflate(R.layout.dialog_bt_conn_user_verification, null);

        //Configuring the pop up dialog
        builder.setView(view)
        //What happens when the user clicks the cancel aka negative button of the dialog
        .setTitle("Enter Verification Key...").setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                popUpDialogListener.applyTexts("cancel"); //Pass a cancel string to the applyTexts method of the pop up dialog listener which will be accessed by the splash screen activity
            }
        })
        //What happens when the user clicks the okay aka positive button of the dialog
        .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                String verificationCode = etVerificationCode.getText().toString();
                popUpDialogListener.applyTexts(verificationCode); //Pass the key to the applyTexts method of the pop up dialog listener which will be accessed by the splash screen activity
            }
        });

        etVerificationCode = view.findViewById(R.id.EditText_Verification_Key);
        return builder.create();
    }

    //Interface for background activity that launched the pop up dialog to access the info that the user entered in the pop up dialog
    public interface PopUpDialogListener {
        void applyTexts(String verificationCode);
    }

    //This onAttach method is called when a fragment attaches itself to an activity...
    //In this case its called when the pop up dialog fragment attaches itself to the main activity
    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        popUpDialogListener = (PopUpDialogListener) context;

    }
}
