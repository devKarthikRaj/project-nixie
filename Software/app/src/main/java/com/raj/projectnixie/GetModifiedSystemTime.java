package com.raj.projectnixie;

import android.annotation.SuppressLint;

import java.text.SimpleDateFormat;
import java.util.Date;

class GetModifiedSystemTime {
    //Get current date from Java Date class
    private final Date date = new Date();

    //Get dateFormat of the type SimpleDateFormat from Java SimpleDateFormat class (used for formatting Date from Date class)
    @SuppressLint("SimpleDateFormat")
    private final SimpleDateFormat SimpleDateFormat = new SimpleDateFormat("MM/dd/yyyy HH:mm");

    //Convert Date to ASCII String using SimpleDateFormat's format method
    private final String stringASCIIDate = SimpleDateFormat.format(date);

    String getCurrentSysMin() {
        // MM/dd/yyyy HH:mm > Minutes are in position 14 and 15
        return stringASCIIDate.charAt(14) + Character.toString(stringASCIIDate.charAt(15));
    }

    String getCurrentSysHour() {
        // MM/dd/yyyy HH:mm > Hours are in position 11 and 12
        return stringASCIIDate.charAt(11) + Character.toString(stringASCIIDate.charAt(12));
    }
}














