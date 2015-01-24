package com.securics.wildlifecapture;

import java.io.IOException;
import java.util.List;

import android.content.Context;
import android.content.SharedPreferences;
import android.hardware.Camera;
import android.hardware.Camera.Size;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback {    	
	private static final String TAG = "CameraPreview";
	
	//public static Camera cam;
	public Camera cam;
	private SurfaceHolder sHolder;
	
	/**
	 * Call initCamera to set camera
	 * @param context
	 * @param cam
	 */
	public CameraPreview(Context context, Camera cam) {
		super(context);
		
		if(cam==null && MainActivity.debugMode) Log.e(TAG,"Camera Not Responding!");
		//CameraPreview.cam = cam;
		this.cam = cam;
		initCamera();
		
		sHolder = this.getHolder();
		sHolder.addCallback(this);				
	}
	
	/**
	 * Set camera parameters to preferred settings
	 */
    private void initCamera() {
    	Camera.Parameters params = cam.getParameters();        	
    	
    	List<Size> sizeList = params.getSupportedPreviewSizes();
    	Size bestPreview = sizeList.get(0);
    	for(Size s : sizeList) {
    		if((s.height > bestPreview.height) && (s.width > bestPreview.width)) {
    			bestPreview = s;
    		}
    	}
    	
    	sizeList = params.getSupportedPictureSizes();
    	Size bestPicture = sizeList.get(0);
    	for(Size s : sizeList) {
    		if((s.height > bestPicture.height) && (s.width > bestPicture.width)) {
    			bestPicture = s;
    		}
    	}
    	SharedPreferences settings = PreferenceManager
				.getDefaultSharedPreferences(getContext());

    	if(MainActivity.debugMode)
    	{
    		Log.i(TAG, "FocusMode: " + settings.getString("FocusMode", "auto"));
    		Log.i(TAG, "SceneMode: " + settings.getString("SceneMode", "auto"));
    		Log.i(TAG, "White Balande: " + settings.getString("WBalanceMode", "auto"));
    	}
    	params.setFocusMode(settings.getString("FocusMode", "auto"));
    	params.setSceneMode(settings.getString("SceneMode","auto"));
    	params.setWhiteBalance(settings.getString("WBalanceMode","auto"));
    	params.setPreviewSize(bestPreview.width, bestPreview.height);
    	params.setPictureSize(bestPicture.width, bestPicture.height);
    	
    	cam.setParameters(params);    	
    }

    /**
     * previews displau of camera is on
     */
	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		if (cam != null) {
			if(MainActivity.debugMode) Log.i("CameraPreview", "Surface Created");
			try {
				cam.setPreviewDisplay(sHolder);
				cam.startPreview();
			} catch (IOException e) {
				if(MainActivity.debugMode) Log.e(TAG, "Cannot Set Preview Display", e);
			}
		}
	}

	/**
	 * previews if camera is on
	 */
	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		if(MainActivity.debugMode) Log.i("CameraPreview","Surface Changed");
		if(cam != null) {
			try {
				cam.setPreviewDisplay(holder);
			} catch (Throwable t) {
				if(MainActivity.debugMode) Log.e("CameraPreview", "Exception in surface changed: " + t);
			}
			cam.startPreview();
		}
	}

	/**
	 * closes camera
	 */
	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		if(MainActivity.debugMode) Log.i("CameraPreview", "Surface Destroyed");
     	
		if(cam != null) {
     		if(MainActivity.debugMode) Log.i("CameraPreview", "Cam NOT NULL in Surface Destroyed");
     		cam.stopPreview();
     		cam.setPreviewCallback(null);
     		cam.cancelAutoFocus();
     		cam.release();
			
     	}
    	cam = null;
    		
	}
}