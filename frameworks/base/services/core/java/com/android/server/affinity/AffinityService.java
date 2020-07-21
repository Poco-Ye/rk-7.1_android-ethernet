package com.android.server.affinity;
import android.os.IBinder;
import android.content.Intent;
import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;


import android.os.IAffinityManager;
import android.os.AffinityManager;

import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.os.RemoteException;
import android.os.Message;
import android.os.Handler;


public class AffinityService extends IAffinityManager.Stub {

    static {
        /*
         * Load the library.  If it's already loaded, this does nothing.
         */
        //zj rm
        //System.loadLibrary("rockchip_fm_jni");
    }
    private static final String TAG = "AffinityService";
    private static final boolean DEBUG = false;
    private static final long delaytime = 3000;
    private static void LOG(String msg) {
        if ( DEBUG ) {
            Log.d(TAG, msg);
        }
    }


    private Context mContext;
    private static final int SET_AFFINITY = 1;
    private static Handler mPolicyHandler;
    private Listener binderListener;

    public static IBinder  mBinder;

    private class PolicyHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case SET_AFFINITY:

            }
        }
    }
    private Runnable mAffinityTask = new Runnable() {
        public void run() {
            LOG("AffinityTask runing");
            int result = init_native();

            if ( result >= 0 ) {
				if(DEBUG)
                Log.d(TAG, "mAffinityTask-->bindThreadToCpu45_native");
                bindThreadToCpu45_native();
            } else {
                if(DEBUG)
                Log.e(TAG, "get affinity configuration fail");
            }

        }
    };
    private final class Listener implements IBinder.DeathRecipient {

        public void binderDied() {
            mBinder.unlinkToDeath(binderListener, 0);
            binderListener = null;
        }
    }

    public static IBinder getBinder() {
        return mBinder;
    }

    /*-------------------------------------------------------*/

    /**
     * Ctor.
     */
    public AffinityService(Context context) {
        LOG("AffinityService() : AffinityService starting!.");
        mContext = context;
        mPolicyHandler = new PolicyHandler();
        return;
    }

    public int init() {
        mPolicyHandler.postDelayed(mAffinityTask,delaytime);
        return 0;
    }

    public void bindToCpu(int cpu) {
        bindToCpu_native(cpu);
    }

    public void bindProcessToCpu45() {
        bindProcessToCpu45_native();
    }

    public void bindThreadToCpu45() {
        bindThreadToCpu45_native();
    }

    public native int init_native();
    public native void bindToCpu_native(int cpu);
    public native void bindProcessToCpu45_native();
    public native void bindThreadToCpu45_native();
}


