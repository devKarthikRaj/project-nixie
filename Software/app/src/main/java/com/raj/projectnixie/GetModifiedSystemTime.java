package com.raj.projectnixie;

import android.annotation.SuppressLint;
import android.widget.Toast;

import java.text.SimpleDateFormat;
import java.util.Date;

class GetModifiedSystemTime {
    //Get current date from Java Date class
    private Date date = new Date();

    //Get dateFormat of the type SimpleDateFormat from Java SimpleDateFormat class (used for formatting Date from Date class)
    @SuppressLint("SimpleDateFormat")
    private SimpleDateFormat SimpleDateFormat = new SimpleDateFormat("MM/dd/yyyy HH:mm");

    //Convert Date to ASCII String using SimpleDateFormat's format method
    private String stringASCIIDate = SimpleDateFormat.format(date);

    String getCurrentSysMin() {
        // MM/dd/yyyy HH:mm > Minutes are in position 14 and 15
        String sysMin = Character.toString(stringASCIIDate.charAt(14))+ Character.toString(stringASCIIDate.charAt(15));
        return sysMin;
    }

    String getCurrentSysHour() {
        // MM/dd/yyyy HH:mm > Hours are in position 11 and 12
        String sysHour = Character.toString(stringASCIIDate.charAt(11)) + Character.toString(stringASCIIDate.charAt(12));
        return sysHour;
    }

    String getCurrentDay() {
        // MM/dd/yyyy HH:mm > Days are in position 3 and 4
        return Character.toString(stringASCIIDate.charAt(3)) + Character.toString(stringASCIIDate.charAt(4));
    }

    String getCurrentMonth() {
        // MM/dd/yyyy HH:mm > Month are in position 0 and 1
        return Character.toString(stringASCIIDate.charAt(0)) + Character.toString(stringASCIIDate.charAt(1));
    }
    String getCurrentYear() {
        // MM/dd/yyyy HH:mm > Years are in position 6, 7, 8 & 9
        return Character.toString(stringASCIIDate.charAt(6)) + Character.toString(stringASCIIDate.charAt(7)) + Character.toString(stringASCIIDate.charAt(8)) + Character.toString(stringASCIIDate.charAt(9));
    }

}














