package com.sscctv.seeeyes;

import android.os.ParcelFileDescriptor;
import android.util.Log;

//import com.iotok.android.board.service.lib.BoardManager;
//import com.iotok.android.board.service.lib.GpioPin;

import com.sscctv.seeeyes.video.VideoInput;

import java.io.IOException;

/**
 * Created by trlim on 2016. 2. 25..
 * <p>
 * Allows Apps to notify its use of specific video input device.
 * Every video input apps must call methods of this API in their onResume/onPause handlers
 * for proper operation.
 */
@SuppressWarnings("JniMissingFunction")
public class VideoSource {

    public static final String SDI = "SDI";
    public static final String HDMI = "HDMI";
    public static final String AUTO = "AUTO";

    public static final String AHD = "AHD";
    public static final String TVI = "TVI";
    public static final String CVI = "CVI";
    public static final String CVBS = "CVBS";

    public static final String LOSS = "LOSS";
    /**
     * Ethernet source
     */
    private static final String ETHERNET = "ETHERNET";

    private String _sourceId;
    private String videoType;

    private ParcelFileDescriptor dispFileDescriptor;
//    private static final int VP_CTRL = 167 + 6;
//    private static final int PSE_EN = 167 + 8;
//    private static final int PD_GOOD = 167 + 13;
//    private static final int ADC_SEL = 167 + 26;

//    private static boolean pd_Status = false;


//    private GpioPin _vpCtrlPin;
//    private GpioPin _pseEnPin;
//    private GpioPin _pdGoodPin;
//    private GpioPin _adcSelPin;

    /**
     * @param sourceId Video source name
     * @throws IOException
     */
    public VideoSource(final String sourceId) {
            _sourceId = sourceId;
//        Log.d("VideoSource", "VideoSource sourceID: " + _sourceId);

//        dispFileDescriptor = native_display_open("/dev/disp");
    }


    /**
     * Return video source id
     *
     * @return id of video source
     */

    public String getType() {
        return videoType;
    }

    public String getSourceId() {
        return _sourceId;
    }

    public boolean is(final String sourceId) {
        return _sourceId.equals(sourceId);
    }

    /**
     * Start using resources for a video input.
     * <p>
     * Call this method in your apps onResume().
     *
     * @throws IOException
     */
    public void acquire() throws IOException {
        switch (_sourceId) {
            case SDI:
//                _vpCtrlPin.setLevel(true);      // HI:OFF, LO:ON
//                _pseEnPin.setLevel(false);      // HI:ON, LO:OFF
//                _adcSelPin.setLevel(true);      // HI:SDI, LO:IPC
                break;

            case ETHERNET:
//                pd_Status = _pdGoodPin.getLevel();
//                if(!pd_Status)
//                _vpCtrlPin.setLevel(false);
//                _pseEnPin.setLevel(true);
//                _adcSelPin.setLevel(false);
                break;

            default:
                setDefaults();
                break;
        }
    }

    /**
     * Stop using resources for a video input.
     * <p>
     * Call this method in your apps onPause().
     *
     * @throws IOException
     */
    public void release() throws IOException {
        setDefaults();
    }

    public void close() throws IOException {
//        _vpCtrlPin = null;
//        _pseEnPin = null;
//        _adcSelPin = null;

        try {
            dispFileDescriptor.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        dispFileDescriptor = null;
    }

    private void setDefaults() throws IOException {
//        _vpCtrlPin.setLevel(true);
//        _pseEnPin.setLevel(false);
//        _adcSelPin.setLevel(false);
    }

    public void set48vPowerOn() throws IOException {
//        _vpCtrlPin.setLevel(false);
    }

    public void set48vPowerOff() throws IOException {
//        _vpCtrlPin.setLevel(true);
    }

    public void setCvbsOut(boolean tv_out) {
//        Log.d(TAG, "sendCommand(" + (int) command + ")");

        sendCvbsOutIoctlKeyCommand(tv_out);
    }

    private void sendCvbsOutIoctlKeyCommand(boolean tv_out) {
        try {
            native_display_cvbs_out(dispFileDescriptor.getFd(), tv_out);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private native ParcelFileDescriptor native_display_open(String path);

    private native int native_display_cvbs_out(int fd, boolean tv_out) throws IOException;

    static {
        try {
            System.loadLibrary("seeeyes-lib");
        } catch (UnsatisfiedLinkError ule) {
            System.err.println("WARNING: VideoSource Could not load seeeyes library!");
        }
    }
}
