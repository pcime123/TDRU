package com.sscctv.seeeyes.video;

import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;

import com.sscctv.seeeyes.SysFsMonitor;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;

/**
 * Created by trlim on 2015. 12. 7..
 * <p>
 * SDI 입력을 다루는 클래스
 */
public class SdiInput extends CameraInput {
    private static final String TAG = "SdiInput";

    public static final String ARG_MODE = "sdi_mode";

    public static final int MODE_STOP = '0';  //48
    public static final int MODE_HD = '1';   //49
    public static final int MODE_3G = '2';   //51
    public static final int MODE_EX1 = '3';// 52
    public static final int MODE_EX_3G = '4';// 53
    public static final int MODE_EX2 = '5';// 54
    public static final int MODE_EX_4K = '6';// 55
    public static final int MODE_EX_TDM = '7';// 56
    public static final int MODE_LOSS = '8';
    //public static final int MODE_UNSTABLE = '5';
//    public static final int

    private final SysFsMonitor mSignalMonitor;
    //private boolean mForceNotifySignalChange = false;

    public SdiInput(SurfaceView surfaceView, Listener listener) {
        super(VideoInput.VIDEO_INPUT_SDI, surfaceView, listener);

        mSignalMonitor = new SysFsMonitor("/sys/class/sdi/en332/signal");

//        mSignalMonitor = new SysFsMonitor("/sys/class/sdi/ex01a/signal");

        //setMode(MODE_STOP);
        HdmiInput.disable();
        AnalogInput.disable();
    }

    @Override
    public void start(Bundle args) {
        super.start(args);

        int mode = MODE_HD;
        if (args != null) {
            mode = args.getInt(ARG_MODE, MODE_HD);
        }

        setMode(mode);
        setMode28xx(getInput());
//        setMode28xx(true);

        Log.d(TAG, "Getinput: " + getInput());

        try {
            //mForceNotifySignalChange = true;

            mSignalMonitor.start(new SysFsMonitor.AttributeChangeHandler() {
                @Override
                public void onAttributeChange(RandomAccessFile file) {
                    byte[] value = new byte[16];
                    int length = 0;
                    try {
                        length = file.read(value);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    if (length > 0) {
                        String signal = new String(value).substring(0, length);

                        switch (signal) {
                            case "nosignal":
                                notifySignalChange(false, 0, 0, '?', 0.0F, MODE_STOP, 254);
                                if (!isRecording()) {
                                    //stopPreview();
                                    stopCameraInput();
                                }

                                //mForceNotifySignalChange = false;
                                break;

                            case "changed":
                                //if (getCamera() != null)    stopCameraInput();
                                startCameraInput();
                                startPreview();
                                notifySignalChange(true, getWidth(), getHeight(), getScan(), getRate(), getMode(), 254);

                                //mForceNotifySignalChange = false;
                                break;

                            default:
                                //if (mForceNotifySignalChange) {
                                notifySignalChange(true, getWidth(), getHeight(), getScan(), getRate(), getMode(), 254);

                                //stopAndStartPreview();

                                //mForceNotifySignalChange = false;
                                //}
                                break;
                        }
                    }
                }
            });
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void stop() {
        Log.d(TAG, "SDI Mode Stop");

        try {
            mSignalMonitor.stop();
        } catch (IOException e) {
            e.printStackTrace();
        }

        disable();

        super.stop();
    }

    public static void disable() {
        setMode(MODE_STOP);
        setMode28xx(-1);
//        setMode28xx(false);


    }

    public static void setMode(int mode) {

        try {
            FileOutputStream file = new FileOutputStream("/sys/class/sdi/en332/run");
            file.write(mode);
            file.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static void setMode28xx(int input) {
        String value;
        Log.d(TAG, "getinput: " + input);
        switch (input) {
            case VIDEO_INPUT_SDI:
                value = "6";
                break;
            default:
                value = "0";
                break;
        }
        try {
            Log.d(TAG, "getvlaue: " + value);
            FileOutputStream file = new FileOutputStream("/sys/class/misc/tp28xx/run");
            file.write(value.getBytes());
            file.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

//
//    private static void setMode28xx(boolean value) {
//        try {
//            FileOutputStream file = new FileOutputStream("/sys/class/misc/tp28xx/run");
//            file.write((value ? "6" : "0").getBytes());
//            file.close();
//        } catch (IOException e) {
//            e.printStackTrace();
//        }
//    }

    public int getMode() {
        int mode = 0;
        try {
            FileInputStream file = new FileInputStream("/sys/class/sdi/en332/mode");
            mode = file.read();
            file.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        Log.d(TAG, "GetMode= " + mode);
        return mode;
    }

    public int getWidth() {
        int width = 0;
        try {
            FileInputStream file = new FileInputStream("/sys/class/sdi/en332/width");
            byte[] value = new byte[16];
            int length = file.read(value);
            if (length > 0) {
                width = Integer.parseInt(new String(value).substring(0, length));
            }
            file.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return width;
    }

    @Override
    public int getHeight() {
        int height = 0;
        try {
            FileInputStream file = new FileInputStream("/sys/class/sdi/en332/height");
            byte[] value = new byte[16];
            int length = file.read(value);
            if (length > 0) {
                height = Integer.parseInt(new String(value).substring(0, length));
            }
            file.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return height;
    }

    public char getScan() {
        char scan = '?';
        try {
            FileInputStream file = new FileInputStream("/sys/class/sdi/en332/rate");
            scan = (char) file.read();
            file.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return Character.toLowerCase(scan);
    }

    public float getRate() {
        float rate = 0;
        try {
            FileInputStream file = new FileInputStream("/sys/class/sdi/en332/rate");
            byte[] value = new byte[16];
            int length = file.read(value);
            if (length > 1) {
                rate = Float.parseFloat(new String(value).substring(1, length));
            }
            file.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return rate;
    }

    public int getStd() {
        int std = 0;
        try {
            FileInputStream file = new FileInputStream("/sys/class/misc/tp28xx/std");
            byte[] value = new byte[16];
            int length = file.read(value);
            if (length > 0) {
                std = Integer.parseInt(new String(value).substring(0, length));
            }
            file.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return std;
    }


    public final class CrcCounts {
        public final int xcrc;
        public final int ycrc;
        public final int ccrc;

        public CrcCounts(int xcrc, int ycrc, int ccrc) {
            this.xcrc = xcrc;
            this.ycrc = ycrc;
            this.ccrc = ccrc;
        }
    }

    public CrcCounts getCrcCounts() {
        CrcCounts counts = new CrcCounts(0, 0, 0);
        try {
            FileInputStream file = new FileInputStream("/sys/class/sdi/en332/crc");
            byte[] value = new byte[32];
            int length = file.read(value);
            if (length > 1) {
                String[] splited = new String(value, 0, length).split("\\s+");
                counts = new CrcCounts(
                        Integer.parseInt(splited[0]),
                        Integer.parseInt(splited[1]),
                        Integer.parseInt(splited[2]));
            }
            file.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return counts;
    }


    public void resetCrcCounts() {
        try {
            FileOutputStream file = new FileOutputStream("/sys/class/sdi/en332/crc");
            file.write('0');
            file.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}
