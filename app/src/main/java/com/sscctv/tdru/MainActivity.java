package com.sscctv.tdru;

import android.annotation.SuppressLint;
import android.app.Activity;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;

import android.telecom.Call;
import android.util.Log;

import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;

import android.view.Window;
import android.view.WindowManager;
import android.widget.AdapterView;

import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;

import com.sscctv.seeeyes.SpiPort;

import java.io.FileInputStream;

import java.io.IOException;
import java.util.Objects;


public class MainActivity extends Activity implements View.OnClickListener, CompoundButton.OnCheckedChangeListener {
    private static final String TAG = "TDRU_1904151935";
    private TextView stat1, stat2, stat3, stat4, len1, len2, len3, len4;
    private TextView ref1, ref2, ref3, ref4, amp1, amp2, amp3, amp4;
    private String[] cmdline = {"sh", "-c", "echo 1 > /sys/class/misc/mv88e6176/vct"};
    private String[] cmdline1 = {"sh", "-c", "echo 2 > /sys/class/misc/mv88e6176/vct"};
    private Runtime runtime = Runtime.getRuntime();
    private Boolean state;
    private int lengthValue1, lengthValue2, lengthValue3, lengthValue4;
    private Button precautions, diagram, start;
    private Context mContext;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

//        Log.i(TAG, "TDRU App Data: 201903260918   Version: 2.0.0");


//        ImageView imageView = findViewById(R.id.cable_image);
//        final Animation animation = AnimationUtils.loadAnimation(this, R.anim.cable_anim);
//        imageView.startAnimation(animation);
//
//        ImageView imageView1 = findViewById(R.id.monitor_image);
//        final AnimationDrawable drawable = (AnimationDrawable) imageView1.getBackground();
//        drawable.start();
        start = findViewById(R.id.start_button);
        start.setFocusable(true);
        start.setFocusableInTouchMode(true);
        start.requestFocus();
        findViewById(R.id.start_button).setOnClickListener(this);
        findViewById(R.id.diagram_button).setOnClickListener(this);
        findViewById(R.id.pre_button).setOnClickListener(this);
//        findViewById(R.id.test1_button).setOnClickListener(this);
        Switch portSwitch = findViewById(R.id.port_switch);
        portSwitch.setOnCheckedChangeListener(this);
        state = portSwitch.isChecked();
        mContext = MainActivity.this;
        try {
            if (state) {
                runtime.exec(cmdline1);
            } else {
                runtime.exec(cmdline);
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
//        Log.d(TAG, "Port State: " + portSwitch.isChecked());

        stat1 = findViewById(R.id.status_1);
        stat2 = findViewById(R.id.status_2);
        stat3 = findViewById(R.id.status_3);
        stat4 = findViewById(R.id.status_4);

        len1 = findViewById(R.id.length_1);
        len2 = findViewById(R.id.length_2);
        len3 = findViewById(R.id.length_3);
        len4 = findViewById(R.id.length_4);

        amp1 = findViewById(R.id.level_1);
        amp2 = findViewById(R.id.level_2);
        amp3 = findViewById(R.id.level_3);
        amp4 = findViewById(R.id.level_4);


    }

    private void openPoeView() {
        Intent sendIntent = new Intent("com.sscctv.poeView");
        sendIntent.putExtra("location", "open");
        sendBroadcast(sendIntent);
    }

    private void closePoeView() {
        Intent sendIntent = new Intent("com.sscctv.poeView");
        sendIntent.putExtra("location", "close");
        sendBroadcast(sendIntent);
    }


    @Override
    protected void onResume() {
        Log.i(TAG, "onResume");
        closePoeView();
        super.onResume();

    }

    @Override
    protected void onPause() {
        Log.i(TAG, "onPause");
        openPoeView();
//        mSpi.close();
        super.onPause();

    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @SuppressLint("SetTextI18n")
    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.start_button:
                CheckTypesTask task = new CheckTypesTask();
                task.execute();


                break;

            case R.id.diagram_button:
                AlertDialog.Builder builder;
                AlertDialog alertDialog;


                LayoutInflater inflater = (LayoutInflater) getApplicationContext().getSystemService(LAYOUT_INFLATER_SERVICE);
                View layout = inflater.inflate(R.layout.custom_dialog, null);
                ImageView imageView = layout.findViewById(R.id.imageView2);
                imageView.setImageResource(R.drawable.image2);
                builder = new AlertDialog.Builder(mContext);
                builder.setView(layout);

                builder.setNegativeButton(getResources().getString(R.string.close), new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                });

                alertDialog = builder.create();

                WindowManager.LayoutParams layoutParams = new WindowManager.LayoutParams();
                layoutParams.copyFrom(alertDialog.getWindow().getAttributes());
                layoutParams.width = 1500;

                alertDialog.show();
                Window window = alertDialog.getWindow();
                window.setAttributes(layoutParams);
                break;
            case R.id.pre_button:
                AlertDialog.Builder builder1;
                AlertDialog alertDialog1;

                LayoutInflater inflater1 = (LayoutInflater) getApplicationContext().getSystemService(LAYOUT_INFLATER_SERVICE);
                View layout1 = inflater1.inflate(R.layout.pre_dialog, null);
                builder1 = new AlertDialog.Builder(mContext);
                builder1.setView(layout1);

                builder1.setNegativeButton(getResources().getString(R.string.close), new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                });

                alertDialog1 = builder1.create();

                WindowManager.LayoutParams layoutParams1 = new WindowManager.LayoutParams();
                layoutParams1.copyFrom(alertDialog1.getWindow().getAttributes());
                layoutParams1.width = 1500;

                alertDialog1.show();
                Window window1 = alertDialog1.getWindow();
                window1.setAttributes(layoutParams1);

                break;
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        state = isChecked;

        try {
            if (state) {
                runtime.exec(cmdline1);
            } else {
                runtime.exec(cmdline);
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @SuppressLint("StaticFieldLeak")
    private class CheckTypesTask extends AsyncTask<Void, Void, Void> {
        ProgressDialog progressDialog = new ProgressDialog(MainActivity.this);


        @Override
        protected void onPreExecute() {
            progressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
            progressDialog.setCanceledOnTouchOutside(false);
            progressDialog.setCancelable(false);
            progressDialog.setMessage(getResources().getText(R.string.please_wait).toString());


            progressDialog.show();
            initSetText();

            super.onPreExecute();

        }

        @Override
        protected Void doInBackground(Void... voids) {

            try {
                if (state) {
                    runtime.exec(cmdline1);
                } else {
                    runtime.exec(cmdline);
                }

            } catch (IOException e) {
                e.printStackTrace();
            }

            return null;
        }

        @Override
        @SuppressLint("DefaultLocale")
        protected void onPostExecute(Void aVoid) {
            Log.d(TAG, "onPostExecute()");
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {

                    lengthValue1 = getData().length1;
                    lengthValue2 = getData().length2;
                    lengthValue3 = getData().length3;
                    lengthValue4 = getData().length4;

//                    Log.d(TAG, "Length Value1: " + lengthValue1 + " Length Value2: " + lengthValue2 + " Length Value3: " + lengthValue3 + " Length Value4: " + lengthValue4);
//                    Log.d(TAG, "Length Value1: " + (lengthValue1 >= 1000) + " Length Value2: " + (lengthValue2 >= 1000) + " Length Value3: " + (lengthValue3 >= 1000) + " Length Value4: " + (lengthValue4 >= 1000));

                    if ((lengthValue1 >= 1000) || (lengthValue2 >= 1000) || (lengthValue3 >= 1000) || (lengthValue4 >= 1000)) {
//                        Log.d(TAG, "true");
                        statusSetText();
                        lengthSetText();
                        ampSetText();

                    } else {
//                        Log.d(TAG, "false");

                        String unmeasurable = getString(R.string.unmeasurable);
//
                        stat1.setText(unmeasurable);
                        stat2.setText(unmeasurable);
                        stat3.setText(unmeasurable);
                        stat4.setText(unmeasurable);

                        len1.setText(R.string.none_length);
                        len2.setText(R.string.none_length);
                        len3.setText(R.string.none_length);
                        len4.setText(R.string.none_length);

                        amp1.setText(R.string.none_length);
                        amp2.setText(R.string.none_length);
                        amp3.setText(R.string.none_length);
                        amp4.setText(R.string.none_length);
                    }

                    progressDialog.dismiss();

                }

            }, 1500);
            getData();


            super.onPostExecute(aVoid);
        }
    }

    public final class ResultValue {
        final int status1;
        final int length1;
        final int status2;
        final int length2;
        final int status3;
        final int length3;
        final int status4;
        final int length4;
        final int refl_pol1;
        final int amplitude1;
        final int refl_pol2;
        final int amplitude2;
        final int refl_pol3;
        final int amplitude3;
        final int refl_pol4;
        final int amplitude4;

        ResultValue(int status1, int length1, int status2, int length2, int status3, int length3, int status4, int length4
                , int refl_pol1, int amplitude1, int refl_pol2, int amplitude2, int refl_pol3, int amplitude3, int refl_pol4, int amplitude4) {
            this.status1 = status1;
            this.length1 = length1;
            this.status2 = status2;
            this.length2 = length2;
            this.status3 = status3;
            this.length3 = length3;
            this.status4 = status4;
            this.length4 = length4;
            this.refl_pol1 = refl_pol1;
            this.amplitude1 = amplitude1;
            this.refl_pol2 = refl_pol2;
            this.amplitude2 = amplitude2;
            this.refl_pol3 = refl_pol3;
            this.amplitude3 = amplitude3;
            this.refl_pol4 = refl_pol4;
            this.amplitude4 = amplitude4;
        }
    }

    public ResultValue getData() {
//        Log.d(TAG, "getData()");
        ResultValue resultValue = new ResultValue(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        try {
            FileInputStream fileInputStream = new FileInputStream("/sys/class/misc/mv88e6176/vct");
            byte[] value = new byte[64];
            int length = fileInputStream.read(value);
            if (length > 1) {
                String[] split = new String(value, 0, length).split("\\s+");
                resultValue = new ResultValue(
                        Integer.parseInt(split[0]),
                        Integer.parseInt(split[1]),
                        Integer.parseInt(split[2]),
                        Integer.parseInt(split[3]),
                        Integer.parseInt(split[4]),
                        Integer.parseInt(split[5]),
                        Integer.parseInt(split[6]),
                        Integer.parseInt(split[7]),
                        Integer.parseInt(split[8]),
                        Integer.parseInt(split[9]),
                        Integer.parseInt(split[10]),
                        Integer.parseInt(split[11]),
                        Integer.parseInt(split[12]),
                        Integer.parseInt(split[13]),
                        Integer.parseInt(split[14]),
                        Integer.parseInt(split[15]));
            }
            fileInputStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
//        Log.d(TAG, "ResultValue: " + resultValue);
        return resultValue;

    }

    public int getStat() {
        int stat = 0;
        try {
            FileInputStream fileInputStream = new FileInputStream("/sys/class/misc/mv88e6176/stat");
            stat = fileInputStream.read();
            fileInputStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return stat;
    }

    private void statusSetText() {
        int value1 = getData().status1;
        int value2 = getData().status2;
        int value3 = getData().status3;
        int value4 = getData().status4;

        String invalid = getString(R.string.invalid);
        String ok = getString(R.string.pair_ok);
        String open = getString(R.string.pair_open);
        String same_short = getString(R.string.same_pair_short);
        String cross_short = getString(R.string.cross_pair_short);
        String busy = getString(R.string.pair_busy);

        int step = 0;

        switch (step) {
            case 0:
                if (value1 == 0) stat1.setText(invalid);
                if (value2 == 0) stat2.setText(invalid);
                if (value3 == 0) stat3.setText(invalid);
                if (value4 == 0) stat4.setText(invalid);
            case 1:
                if (value1 == 1) stat1.setText(ok);
                if (value2 == 1) stat2.setText(ok);
                if (value3 == 1) stat3.setText(ok);
                if (value4 == 1) stat4.setText(ok);
            case 2:
                if (value1 == 2) stat1.setText(open);
                if (value2 == 2) stat2.setText(open);
                if (value3 == 2) stat3.setText(open);
                if (value4 == 2) stat4.setText(open);
            case 3:
                if (value1 == 3) stat1.setText(same_short);
                if (value2 == 3) stat2.setText(same_short);
                if (value3 == 3) stat3.setText(same_short);
                if (value4 == 3) stat4.setText(same_short);
            case 4:
                if (value1 == 4) stat1.setText(cross_short);
                if (value2 == 4) stat2.setText(cross_short);
                if (value3 == 4) stat3.setText(cross_short);
                if (value4 == 4) stat4.setText(cross_short);
            case 5:
                if (value1 == 5) stat1.setText(busy);
                if (value2 == 5) stat2.setText(busy);
                if (value3 == 5) stat3.setText(busy);
                if (value4 == 5) stat4.setText(busy);
                break;
        }
    }


    @SuppressLint("SetTextI18n")
    public void lengthSetText() {


        if (lengthValue1 <= 80) lengthValue1 = 0;
        if (lengthValue2 <= 80) lengthValue2 = 0;
        if (lengthValue3 <= 80) lengthValue3 = 0;
        if (lengthValue4 <= 80) lengthValue4 = 0;

        len1.setText(lengthValue1 / 100 + "." + lengthValue1 % 100);
        len2.setText(lengthValue2 / 100 + "." + lengthValue2 % 100);
        len3.setText(lengthValue3 / 100 + "." + lengthValue3 % 100);
        len4.setText(lengthValue4 / 100 + "." + lengthValue4 % 100);
    }

    @SuppressLint("SetTextI18n")
    public void ampSetText() {
        int value1 = getData().amplitude1;
        int value2 = getData().amplitude2;
        int value3 = getData().amplitude3;
        int value4 = getData().amplitude4;

        if (value1 > 99) value1 = 99;
        if (value2 > 99) value2 = 99;
        if (value3 > 99) value3 = 99;
        if (value4 > 99) value4 = 99;

        double zlz1 = Math.abs(100 - value1);
        double zlz2 = Math.abs(100 - value2);
        double zlz3 = Math.abs(100 - value3);
        double zlz4 = Math.abs(100 - value4);

        double zoz1 = Math.abs(100 + value1);
        double zoz2 = Math.abs(100 + value2);
        double zoz3 = Math.abs(100 + value3);
        double zoz4 = Math.abs(100 + value4);

        double gam1 = zlz1 / zoz1;
        double gam2 = zlz2 / zoz2;
        double gam3 = zlz3 / zoz3;
        double gam4 = zlz4 / zoz4;

        double db1 = 20 * Math.log(gam1) / Math.log(10);
        double db2 = 20 * Math.log(gam2) / Math.log(10);
        double db3 = 20 * Math.log(gam3) / Math.log(10);
        double db4 = 20 * Math.log(gam4) / Math.log(10);

//        Log.d(TAG, "amp1: " + value1 + " zlz1: " + zlz1 + " zoz1: " + zoz1 + " gam1: " + gam1 + " db1: " + db1);
//        Log.d(TAG, "amp2: " + value2 + " zlz2: " + zlz2 + " zoz2: " + zoz2 + " gam2: " + gam2 + " db2: " + db2);
//        Log.d(TAG, "amp3: " + value3 + " zlz3: " + zlz3 + " zoz3: " + zoz3 + " gam3: " + gam3 + " db3: " + db3);
//        Log.d(TAG, "amp4: " + value4 + " zlz4: " + zlz4 + " zoz4: " + zoz4 + " gam4: " + gam4 + " db4: " + db4);
        @SuppressLint("DefaultLocale") String num1 = String.format("%.2f", db1);
        @SuppressLint("DefaultLocale") String num2 = String.format("%.2f", db2);
        @SuppressLint("DefaultLocale") String num3 = String.format("%.2f", db3);
        @SuppressLint("DefaultLocale") String num4 = String.format("%.2f", db4);


        amp1.setText(num1);
        amp2.setText(num2);
        amp3.setText(num3);
        amp4.setText(num4);
    }
//
//    public void refSetText() {
//
//        String pos = "Positive";
//        String neg = "Negative";
//
//        int value1 = getData().refl_pol1;
//        int value2 = getData().refl_pol2;
//        int value3 = getData().refl_pol3;
//        int value4 = getData().refl_pol4;
//
//        int step = 0;
//
//        switch (step) {
//            case 0:
//                if (value1 == 0) ref1.setText(neg);
//                if (value2 == 0) ref2.setText(neg);
//                if (value3 == 0) ref3.setText(neg);
//                if (value4 == 0) ref4.setText(neg);
//            case 1:
//                if (value1 == 1) ref1.setText(pos);
//                if (value2 == 1) ref2.setText(pos);
//                if (value3 == 1) ref3.setText(pos);
//                if (value4 == 1) ref4.setText(pos);
//
//                break;
//        }
//    }

    private void initSetText() {
        stat1.setText(R.string.none);
        stat2.setText(R.string.none);
        stat3.setText(R.string.none);
        stat4.setText(R.string.none);

        len1.setText(R.string.none_length);
        len2.setText(R.string.none_length);
        len3.setText(R.string.none_length);
        len4.setText(R.string.none_length);

        amp1.setText(R.string.none_length);
        amp2.setText(R.string.none_length);
        amp3.setText(R.string.none_length);
        amp4.setText(R.string.none_length);
    }

}


