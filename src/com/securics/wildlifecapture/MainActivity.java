package com.securics.wildlifecapture;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.sql.Date;
import java.text.SimpleDateFormat;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Scanner;
import java.util.Set;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.hardware.Camera;
import android.hardware.Camera.PictureCallback;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.MediaRecorder;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.PowerManager;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Display;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity implements SensorEventListener {

	// Is Program running?
	private static boolean RUNNING = false;
	private static boolean DEATH = false;

	// This variable will eventually represent whether or not several menu items
	// are present under the heading,
	// "Debug Items."
	public static boolean debugMode = true;

	//This represents whether or not the program is allowed to delete everything in the processed images directory if the device comes close to running out of storage.
	//THis is presently disabled for testing purposes.
	public static boolean mayAutoDelete=false;

	// private static final String TAG = "SquirrelGPU_Main";
	private static final String TAG = "WildLifeCaptureMain";// Set to the
															// updated name.

	//This dialog will be dynamically remade to fit various needs.
	private Dialog outDialog;
	//This array of strings will be used to populate the dialog box.
	private String resStringList[];
	private ListView dialogViewList;
	private View view;

	
	SharedPreferences settings;

	private final static int CAM_3D = 0;
	// This is to protect against AutoFocus running before it should.
	private Camera cam = null;
	private CameraPreview camPreview;

	// to log all the debug stuff
	private ActivityDebug mDebug = null;

	private boolean prefsLoaded = false;

	private String DEVICE_ID = "ID";

	private boolean SKIP_JNI = false;

	private File directory = null;
	private File burstCaptureDir = null;

	private long START_HOUR = -1;
	private long START_MINUTE = -1;

	private long END_HOUR = -1;
	private long END_MINUTE = -1;

	private static String LOG_FILE = "";

	public File processingQueueLog = null;

	boolean gaveConfig = false;

	boolean spaceAvailable = false;

	PowerManager.WakeLock wl;

	private Handler PSHandler = new Handler();

	//for the resolution
	String selectedRes;
	
	//TODO What's this?
	int currentMode = 0;
	
	Menu settingsMenu;
	
	//This is so that other threads don't build up an endless mess of picture capture requests.
	private boolean burstIsBusy;

	private int burstNum=2;

	//This is to prevent this swirling mass of threads and processes from getting in each other's way.
	//when trying to use the camera. When it is false, the camera is unlocked. When it is true, a process is using it.
	private boolean cameraLock;

	private Thread autoRunCodeRunner;
	private boolean autoRunCodeMayRun=true;
	//private AutoCodeRunnerRunnable autoCodeRunnable;

	/*
	 * This boolean is checked so that I can meet Task 1:
	 * The camera must, on some sort of event, be able to capture 5 images in succession
	 * An event will toggle willDoCameraBurst, which will cause the camera to take 5 pictures in 
	 * succession.
	 */
	public boolean willDoCameraBurst=false;

	//These are used quite a bit.
	private Thread burstCaptureThread;
	private Thread execBurstCaptureThread;
	private Thread autoFocusThread;
	private Thread JNIThread;
	private Thread CPUInformationThread;
	private boolean burstCanRun;
	private AutoCodeRunnerRunnable autoCodeRunnable;

	
	//Set minimum sizes in real-world units. These, too, are static so that other objects can read them.
	//TODO What units are these in? real-world is not a unit!
	public static double minObjWidth=2;
	public static double minObjHeight=2;
	public static double maxObjWidth=20;
	public static double maxObjHeight=20;
	//User-changeable time interval between captures.
	public static long timeInterval=60000;
	//The next two values appear to be exactly the same.
	public static int numOfPicsPerCapture=2;

	//Declare a great many variables.
	//These are public and static so that they can be readily modified by outside objects.
	//These values are no longer used.
	public static double minPixelObjWidth=800;
	public static double minPixelObjHeight=600;
	public static double maxPixelObjWidth=1920;
	public static double maxPixelObjHeight=1080;

	//private PerspectiveTransformationEngine perspective = 
		//	new PerspectiveTransformationEngine();
	
	// private boolean
	// As per various engine changes, Two photos should be captured and
	// considered at a time.
	private String mostRecentFilename;
	private String[] requestList;
	private int currentRequest = 0;
	private final int REQUESTLISTLENGTH = 2;
	//public static Camera outputCam; 	//Removed because it seems it's never actually used
	public static Camera.Parameters publicParams;
	private Camera.Parameters privateParams;
	public static int publicWidth;
	public static int publicHeight;

	private double currentPitch = 0.0;
	private double cameraVerticalDegreeRange = 0.0;

	//Camera correction calculation data:
	//This is a default height for basic test calculations.
	//This value should actually be converted into pixels later on.
	//Either that or it's in meters. I really don't know!
	public static double camHeightAboveGround=4;
	public static double camMeasuredDegrees;
	private double camVertDegRange;
	private double camHorizDegRange;
	//These will allow pitch information to be displayed in the settings screen, for example.
	public static SensorManager sensorManager;
	public static Sensor pitchSensor;
	//End camera correction calculation data

	//The new stuff or the original/old stuff?	
	private boolean useNewProcess;

	
	private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
		@Override
		public void onManagerConnected(int status) {

			switch (status) {
			case LoaderCallbackInterface.SUCCESS: {
				if (debugMode)
					Log.i(TAG, "OpenCV loaded successfully");

			}
				break;
			default: {
				super.onManagerConnected(status);
			}
				break;
			}
		}
	};

	/**So we can pass the config stuff to JNI*/
	public native int doRevisedJNI(String rawName, String configFileName, String name, 
			double camVertDegRange, double camHorizDegRange, double picSizeX, double picSizeY);
	
	public static native int giveConfig(String key, String value);

	static {
		System.loadLibrary("nativePipeline");
	}


	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		if (debugMode)
			Log.i(TAG, "onCreate()");
		DEATH = false;
		setContentView(R.layout.main);

		settings = PreferenceManager
				.getDefaultSharedPreferences(getApplicationContext());

		if (debugMode)
			Log.i(TAG, "New camera instance");

		if (cam == null) {

			if (debugMode)
				Log.i(TAG, "cam was null");

			cam = getCameraInstance();
		}

		if (cam == null) {
			if (mDebug != null && debugMode)
				mDebug.logMessage(TAG, "Camera is not available onCreate().");
			Toast.makeText(this, "Camera is not available", Toast.LENGTH_LONG)
					.show();
			finish();
		}

		if (!prefsLoaded)
			if (!loadPrefs()) {
				if (debugMode)
					Log.e(TAG, "Error loading stored preferences");
				if (mDebug != null && debugMode)
					mDebug.logMessage(TAG, "Error loading stored preferences.");
			}

		//Set up the checker thread:
		execBurstCaptureThread=new Thread(new BurstCheckerRunnable());
		//Set up the autofocus thread:
		autoFocusThread=new Thread(new AutoFocusRunnable());
		//autoFocusThread.start();
		//Start it up.
		execBurstCaptureThread.start();
		cameraLock=false;
		//autoBaseCodeRunnerMeister=new Handler();
		autoCodeRunnable=new AutoCodeRunnerRunnable();
		JNIThread=new Thread(new PushToJNIRunnable());
		

		requestList=new String[REQUESTLISTLENGTH];
		
		for(int i=0;i<REQUESTLISTLENGTH;i++)
		{
			//Set all of the new strings up to avoid NullPointerExceptions.
			requestList[i]=new String();
		}

		
		if(cam!=null)
		{
			//cameraVerticalDegreeRange=cam.getParameters().getVerticalViewAngle();
		}

		sensorManager=(SensorManager)getSystemService(SENSOR_SERVICE);
		//pitchSensor=sensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR);
		//Yes, I understand that this sensor is depreciated, however I just want to get something usable.
		//I'll use newer, more accurate sensors later.
		pitchSensor=sensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);

		//Removed: useless
		//outputCam=cam;
		
	
		// Set this to the processed files directory.
		processingQueueLog = new File(getRootSDCard() + "/"
				+ this.getResources().getString(R.string.processed_key));

	}

	@Override
	public void onResume() {
		super.onResume();
	
		// Removed: useless
		//if (outputCam == null) {
		//	cam = null;
		//}
		
		loadPrefs();
		
		if (debugMode)
			Log.i(TAG, "onResume()");

		if (debugMode)
			Log.i(TAG, "Got data: " + publicWidth + " by " + publicHeight);

		if (!OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this,
				mLoaderCallback) && debugMode) {
			Log.e(TAG, "Cannot connect to OpenCV Manager");
		}

		if (mDebug == null)  {
			Log.i(TAG,"New Debug Activity: "+LOG_FILE);
			mDebug = new ActivityDebug(LOG_FILE);
		}
		else Log.i(TAG,"mDEBUG LOG: "+LOG_FILE);
		
		mDebug.setID(DEVICE_ID);

		if (debugMode) {
			mDebug.logMessage(TAG, "calling onResume()");
			mDebug.logMessage(TAG, "Device ID set to: " + DEVICE_ID);
		}

		FrameLayout camLayout = (FrameLayout) findViewById(R.id.camera_view);

		if (debugMode) {
			Log.i(TAG, "New camera instance onResume()");
			mDebug.logMessage(TAG, "New camera instance onResume()");
		}

		if (cam == null) {

			if (debugMode) {
				Log.i(TAG, "cam was null");
				mDebug.logMessage(TAG, "Camera null onResume()");
			}
			cam = getCameraInstance();
		}

		if (cam == null) {
			Toast.makeText(this,
					"Camera is not available. Trying to force get...",
					Toast.LENGTH_LONG).show();

			if (debugMode)
				mDebug.logMessage(TAG, "Camera is not available onResume()");
			int max = 30;
			while (cam == null && max-- > 0) {
				cam = getCameraInstance();
			}

			// finish();
		} else {
			if (debugMode)
				mDebug.logMessage(TAG, "Camera acquired onResume()");
		}

		if (debugMode)
			mDebug.logMessage(TAG, "new CameraPreview");

		if (camPreview == null && cam != null) {
			if (debugMode)
				mDebug.logMessage(TAG, "need a CameraPreview object");

			camPreview = new CameraPreview(this, cam);

			if (publicWidth > 0 && publicHeight > 0) {
				// cam.stopPreview();
				Camera.Parameters tempParams;
				tempParams = camPreview.cam.getParameters();
				// This should set the preview so that it is consistent with the
				// selected resolution.
				// tempParams.setPreviewSize(publicWidth, publicHeight);
				tempParams.setPictureSize(publicWidth, publicHeight);
				// tempParams.setPictureSize(1024, 768);

				int maxPreviewWidth, maxPreviewHeight, minPreviewWidth, minPreviewHeight, listLength;
				listLength = tempParams.getSupportedPreviewSizes().size();

				// Get the maximum and minimum supported preview resolutions.
				maxPreviewWidth = tempParams.getSupportedPreviewSizes().get(
						listLength - 1).width;
				maxPreviewHeight = tempParams.getSupportedPreviewSizes().get(
						listLength - 1).height;
				minPreviewWidth = tempParams.getSupportedPreviewSizes().get(0).width;
				minPreviewHeight = tempParams.getSupportedPreviewSizes().get(0).height;

				if (debugMode)
					Log.i(TAG, "Max and min preview sizes: " + maxPreviewWidth
							+ "x" + maxPreviewHeight + ", " + minPreviewWidth
							+ "x" + minPreviewHeight);

				// Ensure that the preview will actually fit.
				if (publicWidth > maxPreviewWidth
						|| publicHeight > maxPreviewHeight) {
					tempParams
							.setPreviewSize(maxPreviewWidth, maxPreviewHeight);
				}

				else if (publicWidth < minPreviewWidth
						|| publicHeight < minPreviewHeight) {
					tempParams
							.setPreviewSize(minPreviewWidth, minPreviewHeight);
				}

				// Set it to the selected resolution.
				else {
					tempParams.setPreviewSize(publicWidth, publicHeight);
				}
				// cam.setParameters(cam.getParameters().setPictureSize(publicWidth,
				// publicHeight));
				camPreview.cam.setParameters(tempParams);
				// cam.startPreview();
			}
			// Calls function that crashes emulator.

			saveCameraParams();

			camLayout.addView(camPreview);
			camLayout.setOnTouchListener(new View.OnTouchListener() {

				@Override
				public boolean onTouch(View v, MotionEvent event) {
					ImageView view = (ImageView) findViewById(R.id.imageView);
					FrameLayout.LayoutParams layoutParams1 = (FrameLayout.LayoutParams) view
							.getLayoutParams();

					switch (event.getActionMasked()) {
					case MotionEvent.ACTION_DOWN:
						showPopupMenu(view);
						break;
					case MotionEvent.ACTION_MOVE:
						int x_cord = (int) event.getRawX();
						int y_cord = (int) event.getRawY();
						/*
						 * Toast.makeText(MainActivity.this, "X: " + x_cord +
						 * " Y: " + y_cord, Toast.LENGTH_SHORT).show();
						 */
						if (x_cord > 1200 && x_cord < 1240 && y_cord > 660
								&& y_cord < 700) {
							// showScarlet();
						}
						layoutParams1.leftMargin = x_cord - 25;
						layoutParams1.topMargin = y_cord - 75;
						view.setLayoutParams(layoutParams1);
						break;
					default:
						break;
					}

					return true;
				}
			});

			if (settings == null) {
				settings = PreferenceManager.getDefaultSharedPreferences(this);
				// Get distance above ground settings.
				camHeightAboveGround = Double.parseDouble(settings.getString(
						"Camera Vertical Position", "5"));
				timeInterval = Long.parseLong(settings.getString(
						"Capture Cycle Interval", "60")) * 1000;
				minObjWidth = Double.parseDouble(settings.getString(
						"minObjWidth", "2"));
				minObjHeight = Double.parseDouble(settings.getString(
						"minObjHeight", "2"));
				maxObjWidth = Double.parseDouble(settings.getString(
						"maxObjWidth", "20"));
				maxObjHeight = Double.parseDouble(settings.getString(
						"maxObjHeight", "20"));
				currentMode = Integer.parseInt(settings.getString(
						"processingMode", "1"));

				if (debugMode) {
					Log.i(TAG, "Previous camHeightAboveGround was: "
							+ camHeightAboveGround);
					Log.i(TAG, "New camHeightAboveGround is: "
							+ camHeightAboveGround);
					// Get the new timer interval settings.
					Log.i(TAG, "Previous timer interval was: " + timeInterval);
					Log.i(TAG, "New timer interval is: " + timeInterval);
					// Get new other settings:
					Log.i(TAG, "New minObjWidth=" + minObjWidth);
					Log.i(TAG, "New minObjHeight=" + minObjHeight);
					Log.i(TAG, "New maxObjWidth=" + maxObjWidth);
					Log.i(TAG, "New maxObjHeight=" + maxObjHeight);
					Log.i(TAG, "New currentMode=" + currentMode);
				}
			}

			else {
				camHeightAboveGround = Double.parseDouble(settings.getString(
						"Camera Vertical Position", "5"));
				timeInterval = Long.parseLong(settings.getString(
						"Capture Cycle Interval", "60")) * 1000;
				minObjWidth = Double.parseDouble(settings.getString(
						"minObjWidth", "2"));
				minObjHeight = Double.parseDouble(settings.getString(
						"minObjHeight", "2"));
				maxObjWidth = Double.parseDouble(settings.getString(
						"maxObjWidth", "20"));
				maxObjHeight = Double.parseDouble(settings.getString(
						"maxObjHeight", "20"));
				currentMode = Integer.parseInt(settings.getString(
						"processingMode", "1"));

				if (debugMode) {
					Log.i(TAG, "Previous camHeightAboveGround was: "
							+ camHeightAboveGround);

					Log.i(TAG, "New camHeightAboveGround is: "
							+ camHeightAboveGround);

					Log.i(TAG, "Previous timer interval was: " + timeInterval);

					Log.i(TAG, "New timer interval is: " + timeInterval);

					// Get new other settings:

					Log.i(TAG, "New minObjWidth=" + minObjWidth);

					Log.i(TAG, "New minObjHeight=" + minObjHeight);// Cheese

					Log.i(TAG, "New maxObjWidth=" + maxObjWidth);

					Log.i(TAG, "New maxObjHeight=" + maxObjHeight);

					Log.i(TAG, "New currentMode=" + currentMode);
				}
			}

			// Start firing sensor events.
			sensorManager.registerListener(this, pitchSensor,
					SensorManager.SENSOR_DELAY_NORMAL);
		}

		if (debugMode)
			mDebug.logMessage(TAG, "Starting camera preview");

		if (cam != null) {

			// cam.startPreview();

			// if(publicWidth>0&&publicHeight>0)
			// {
			// cam.stopPreview();
			// Camera.Parameters tempParams;
			// tempParams=cam.getParameters();
			// //tempParams.setPreviewSize(publicWidth, publicHeight);
			// tempParams.setPictureSize(publicWidth, publicHeight);
			// //cam.setParameters(cam.getParameters().setPictureSize(publicWidth,
			// publicHeight));
			// cam.setParameters(tempParams);
			// cam.startPreview();
			// }

			//Camera.Parameters tempParams = cam.getParameters();
			// Set the focus mode. This obsoletes the autofocus thread and all
			// of the bugs it brings.
			// tempParams.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);
			// cam.setParameters(tempParams);
		}

		// loadDocumentationImage();

	}

	@Override
	public void onPause() {
		super.onPause();
		if (mDebug != null && debugMode)
			mDebug.logMessage("MainActivity", "onPause() was called");
		
		cam = null;
		
		//stop firing sensor events.
		sensorManager.unregisterListener(this);
	}

	@SuppressLint("Wakelock")
	@Override
	public void onDestroy() {
		super.onDestroy();

		RUNNING = false;
		DEATH = true;

		cam = null;

		if (mDebug != null && debugMode) {
			mDebug.logMessage(TAG, "calling onDestroy()");
			mDebug.EndPhotoSnapperDebug();
		}

		if (wl != null) wl.release();
		  
		PSHandler.removeCallbacksAndMessages(null);
		 
		this.finish();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		/**
		 *  Inflate the menu; this adds items to the action bar if it is present.
		 */
		getMenuInflater().inflate(R.menu.settings_menu, menu);
		if(debugMode) {
			menu.setGroupEnabled(R.id.group_debug, true);
		}
		else menu.setGroupEnabled(R.id.group_debug, false);
		
		menu.setGroupEnabled(0, true);
		settingsMenu = menu;
		if (mDebug != null)
			mDebug.logMessage(TAG, "onCreateOptionsMenu()");
		else if(debugMode)
			Log.i(TAG, "onCreateOptionsMenu()");

		return true;
	}

	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		/**
		 * This should handle the settings menu.
		 */
		switch(item.getItemId()) {
		
		case R.id.action_settings:
		
			if (mDebug != null)
				mDebug.logMessage(TAG,
						"onOptionsItemSelected (settings was clicked)");
			else if(debugMode)
				Log.i(TAG, "onOptionsItemSelected (settings was clicked)");

			Intent settingsIntent = new Intent(this, SettingsActivity.class);
			MainActivity.this.startActivityForResult(settingsIntent, 1);
			break;
		
		case R.id.action_resActivity:

			//TODO This is still broken. oh well for now...
			//  This should just bring up a dialog box with resolutions.
			//  An activity is a BAD thing here..
			Intent startDebugResActivity = new Intent(this, ChangeResolutionActivity.class);
			MainActivity.this.startActivityForResult(startDebugResActivity, 1);
			break;
			
			
		case R.id.action_startStop:

			startStop();
			break;
		
	
		case R.id.action_help:

			Intent helpIntent=new Intent(this, Documentation.class);
			MainActivity.this.startActivityForResult(helpIntent, 1);
			break;
			
			
		case R.id.action_procImageLog:
			
			//This reinstantiates the file to ensure that any checks or reads on the file are completely accurate.
			//processingQueueLog=new File(getResources().getString(R.string.gallery_default)+"/queue.log");
			
			if(debugMode)
				mDebug.logMessage(TAG, "Processing Image Log");
			
			RUNNING = true;
			
			if(processingQueueLog.exists())
			{
				Toast.makeText(MainActivity.this, "Starting Image Processing Job...", Toast.LENGTH_SHORT);
				int total=0;
				ProgressDialog progressDialog=ProgressDialog.show(MainActivity.this, "Processing...", "Processing...");
				progressDialog.setMessage("Working...");
				//Progress is represented as an integer value from 1 to 10,000.
				
				try {
					Scanner fileInput=new Scanner(processingQueueLog);
					String curItem;
					
					//This step will pre-process the file so that the progress bar will be accurate.
					while(fileInput.hasNextLine())
					{
						fileInput.nextLine();
						total++;
					}
					
					//Reset the scanner:
					fileInput=new Scanner(processingQueueLog);
					
					for(int i=0; fileInput.hasNextLine(); ++i)
					{
						//The formatting for the log file is very simple: raw filenames with a complete path.
						//This may lead to some problems if the users decide to use a separate device to do the processing.
						curItem=fileInput.nextLine();
						progressDialog.setMessage("Current File: "+curItem);
						pushStr(curItem);
						
						//This should probably work in all cases.
						progressDialog.setProgress(Math.round(i/(float)total*10000));
						
						break;
					}
					
					try {
						Thread.sleep(1000);
					} catch (InterruptedException e) {
					}
					
					progressDialog.cancel();
					
					//Push the current processing queue log into a backup file.
					processingQueueLog.renameTo(new File(getRootSDCard() +"/"+processingQueueLog.getAbsolutePath()+"/"+"processingqueue.log.old"));
					
				} catch (FileNotFoundException e) {
					Log.e(TAG,"Can't find: "+processingQueueLog);
					e.printStackTrace();
				}
			}
			
			break;
		
		case R.id.action_setRes:
			
			runResolutionDialog(cam.getParameters());
			break;
		
		case R.id.action_displayRam: 

			//Part of this function is to display a toast with RAM information.
			fitRAM();
			break;
			
		case R.id.action_minLenWidth:

			runMinLengthWidthDialog();
			break;
	
		case R.id.action_maxLenWidth:

			runMaxLengthWidthDialog();
			break;
		
		case R.id.action_camHeight:

			//This is for the distance above ground dialog box.
			runVertDialog();
			break;
		
		case R.id.action_calibrate:

			//This can only be run AFTER some sort of burst capture was started.

			//Set the test image type to checker board.
			//perspective.setCalibrationType(PerspectiveTransformationEngine.CALIBRATION_CHECKERBOARD);
			//Run the calibration for two captures. 
			
			if(debugMode)
				System.err.println("Filenames: "+ requestList[0]+", "+requestList[1]);
			//perspective.runCalibration(requestList[0]);
			//perspective.runCalibration("input.jpg");
			//perspective.runCalibration(requestList[1]);
			break;
	
		case R.id.action_imageComp:
			
			//This does image correction for a set image called "image.jpg"
			//This is to speed up testing of my area algorithm, as
			//The tablet has a faster CPU than my desktop PC. :)
		
		//	perspective.setDegPointedAtGround(38);
		//	perspective.setDistAboveGround(17);
		//	perspective.setVerticalDegRangeOfCamera(40.74);
		//	perspective.runTransform(directory.getAbsolutePath()+"/input.jpg", 
		//			directory.getAbsolutePath()+"/output.jpg");
			break;

		case R.id.action_toggleOld:

			useNewProcess=(!useNewProcess);
			
			if(debugMode)
				Log.i(TAG, "useNewProcess is now: "+useNewProcess);
			
			MainActivity.this.setTitle(getResources()
					.getString(R.string.app_name)
					+ "; oldMethod="+(!useNewProcess));
			break;
			
		case R.id.action_runVidCap:

			MainActivity.this.setTitle(getResources()
					.getString(R.string.app_name)+"Capturing Video.");
			//Capture 10 seconds of video.
			captureVideo(10000);
			MainActivity.this.setTitle(getResources()
					.getString(R.string.app_name)+"Done.");

			break;
	
		//case R.id.:
			//Check for resolution changer.

			//publicParams=outputCam.getParameters();
			//Intent resolutionIntent=new Intent(this, ChangeResolutionActivity.class);
			
			//MainActivity.this.startActivityForResult(resolutionIntent, 1);
			//break;
		
		case R.id.action_changeFreq:
			//Check for frequency changer.
			changeFreqDialog();
			break;
			
	
		case R.id.action_burstTest:
			//This should handle the burst menu item.
			//willDoCameraBurst=true;
			if(debugMode)
			{
				Log.i(TAG, "willDoCameraBurst set to true.");
				Log.i(TAG, "Calling capture method...");
			}
			
			willDoCameraBurst=true;
			break;
		
		default:
			break;
		
		}
		
		item=null;
		return super.onOptionsItemSelected(item);
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {

		if(debugMode)
			Log.i(TAG, "onActivityResult(" + requestCode + ", " + resultCode + ")");
		
		if (requestCode == 1) {
			cam = null;
			camPreview = null;

			// now pick up any changes from prefs
			loadPrefs();
			sendConfigToJNI();

		}
	}

	private void startStop() {

		if (!RUNNING) {
			
			Toast.makeText(MainActivity.this, "Tap Screen to Stop", Toast.LENGTH_LONG).show();
			if(cam != null) {
			MainActivity.this.setTitle(getResources()
					.getString(R.string.app_name)
					+ " - RUNNING at "+cam.getParameters().getPictureSize().width+"x"+cam.getParameters().getPictureSize().height);
			}
			else {
				MainActivity.this.setTitle(getResources()
					.getString(R.string.app_name)
					+ "Running, Could Not Get Res.");
			}

			//This will run the program in either capture or capture and process mode.
			if(currentMode!=3)
			{
				settingsMenu.setGroupEnabled(0, false);
				if(!RUNNING)
					AutoRunCode();
				
				RUNNING = true;
				String key = getApplicationContext().getResources()
						.getString(R.string.appRunning_key);
				Editor editor = settings.edit();
				editor.putBoolean(key, RUNNING);
				editor.commit();
			}
			
			//This will process all of the images listed in the processing log.
			if(currentMode==3)
			{
				//This reinstantiates the file to ensure that any checks or reads on the file are completely accurate.
				processingQueueLog=new File(getRootSDCard() + "/" + 
						getResources().getString(R.string.gallery_default)+"/queue.log");
				
				RUNNING = true;
				String key = getApplicationContext().getResources()
						.getString(R.string.appRunning_key);
				Editor editor = settings.edit();
				editor.putBoolean(key, RUNNING);
				editor.commit();
				
				if(processingQueueLog.exists())
				{
					Toast.makeText(MainActivity.this, "Starting Image Processing Job...", Toast.LENGTH_SHORT);
					int total=0;
					ProgressDialog progressDialog=ProgressDialog.show(MainActivity.this, "Processing...", "Processing...");
					progressDialog.setMessage("Working...");
					//Progress is represented as an integer value from 1 to 10,000.
					//progressDialog.setProgress(value)
					
					try {
						Scanner fileInput=new Scanner(processingQueueLog);
						String curItem;
						
						//This step will pre-process the file so that the progress bar will be accurate.
						while(fileInput.hasNextLine())
						{
							fileInput.nextLine();
							total++;
						}
						
						//Reset the scanner:
						fileInput=new Scanner(processingQueueLog);
						
						for(int i=0;fileInput.hasNextLine();++i)
						{
							//fsda
							//The formatting for the log file is very simple: raw filenames with a complete path.
							//This may lead to some problems if the users decide to use a separate device to do the processing.
							curItem=fileInput.nextLine();
							progressDialog.setMessage("Current File: "+curItem);
							pushStr(curItem);
							
							//This should probably work in all cases.
							progressDialog.setProgress(Math.round(i/(float)total*10000));
						}
					} catch (FileNotFoundException e) {
						Log.e(TAG,"File not found: "+processingQueueLog);
						e.printStackTrace();
					}
				}
				
				//Stop processing.
				RUNNING = false;
				key = getApplicationContext().getResources()
						.getString(R.string.appRunning_key);
				editor = settings.edit();
				editor.putBoolean(key, RUNNING);
				editor.commit();
			}

		} else if (RUNNING) {
			RUNNING = false;
			MainActivity.this.setTitle(getResources()
					.getString(R.string.app_name));
			settingsMenu.setGroupEnabled(0, true);
			PSHandler.removeCallbacksAndMessages(null);

			String key = getApplicationContext().getResources()
					.getString(R.string.appRunning_key);
			Editor editor = settings.edit();
			editor.putBoolean(key, RUNNING);
			editor.commit();

			autoRunCodeMayRun=false;
		}

		
	}
	
	/**
	 * This function is intended to be used by the automatic parsing function. Since there is no need for this
	 * to share any processing resources with the rest of the application, this is not intended to be used as a thread.
	 */
	private void pushStr(String fName)	{
			
		if(debugMode) {
			mDebug.logMessage(TAG, "pushStr: "+fName);

		}
			
		if (DEATH) {
			mDebug.logMessage(TAG, "Death was called.");
			return;
		}
		else {

			for(int i=0;i<REQUESTLISTLENGTH;i++) {
				
				File testFile=new File(directory.getAbsolutePath()+"/"+requestList[i]);
						
				if(!testFile.exists()||testFile.length()==0) {
					//Force stop the method if the file in question doesn't actually exist.
					return;
				}
			}
					
			//Now that we know these files actually exist!
			File file1=new File(fName);
			//File file2=new File(directory.getAbsolutePath()+"/"+requestList[1]);
					
			String file1RawName=fName;
			//String file2RawName=file2.getName().substring(0, file2.getName().lastIndexOf("."));

			int n = 0;
			// Always true.
			if (useNewProcess || !useNewProcess) {

				// This is the new new new process!
				// These functions calculate the area, not the width and height.
				if (debugMode)
					mDebug.logMessage(TAG,
							"Pushing to JNI using the new image pushing process...\n");

			//	perspective.setMinArea(minObjWidth * minObjHeight);
			//	perspective.setMaxArea(maxObjHeight * maxObjWidth);
			//	perspective.setCamHeight(camHeightAboveGround);
			//	perspective.setCamPitch(currentPitch);
				n = doRevisedJNI(file1RawName, null, file1
						.getPath(), cam.getParameters().getVerticalViewAngle(),
						cam.getParameters().getHorizontalViewAngle(), cam
								.getParameters().getPictureSize().width, cam
								.getParameters().getPictureSize().height);

			}
			else {
				// I really should set this up so that it does both of the
				// elements in the file push queue.
				
				//TODO does this need to be if the above if statement is always true?
				//n = l.oldPushImage(file1RawName, null, file1.getPath());
			}

			// Check for capture condition.
			if (n > 50) {
				// Capture 10 seconds of video when the JNI code found something
				// interesting.
				
				//TODO put this back when you're ready
				//captureVideo(10000);
				
				// burstNum=5;
				// willDoCameraBurst=true;
			}

			if (debugMode)
				Log.i(TAG, "JNI Returned: " + n);
		}
	}

	
	/**
	 * This code is to be modified to complete task 1.
	 */
	private void AutoRunCode() {

		autoRunCodeMayRun=true;
		autoRunCodeRunner=new Thread(new AutoCodeRunnerRunnable());
		autoRunCodeRunner.start();

	}

	
	public class AutoCodeRunnerRunnable implements Runnable
	{
		public void run()
		{
			while(autoRunCodeMayRun)
			{
			//Inform the programmer that the program is about to die.
			//I have yet to see this message, however safety is good.
			if(DEATH)
			{
				if(debugMode) Log.i(TAG, "Program dying. Not posting back to handler.");
				autoRunCodeMayRun=false;
			}
			
			else
			{
				
				Log.i(TAG,"burstNum: "+burstNum);
				Log.i(TAG,"burstIsBusy: "+burstIsBusy);
				Log.i(TAG,"CurrentMode: "+currentMode);
				Log.i(TAG,"cameraLock: "+cameraLock);
				
				//burstNum=ctrMax;
				//Just in case JNI found something interesting.
				if(burstNum>2&&burstIsBusy)
				{
					try {
						Thread.sleep(2000);
					} catch (InterruptedException e) {
						if(debugMode) e.printStackTrace();
					}
					
					burstNum=(int) 2;
				}
				
				//This is if the burst isn't actually running, but still has a higher burst number.
				else if(burstNum>2&&!burstIsBusy)
				{
					burstNum=2;
				}
				
				if(cam!=null)
				{
					willDoCameraBurst=true;
					//Sleep for a while in order to give the checker thread some time to pick up the change.
					try {
						Thread.sleep(10);
					} catch (InterruptedException e2) {
					}
					
					//Sleep to allow the camera to save the file to storage for the PushToJNIRunnable.
					try {
						Thread.sleep(2000);
					} catch (InterruptedException e1) {
						Log.e(TAG,"Error on sleep");
					}
					
					//If the mode is set to "Capture Only", then prevent the PushToJNIRunnable from running.
					if(currentMode==2)
					{
						//This will put the photo's filename into the log so that it can be processed by the user later on. 
						if(debugMode) Log.i(TAG, "Logged Filenames: "+requestList[0]+", "+requestList[1]);
						
						logPhoto(requestList[0]);
						logPhoto(requestList[1]);
					}
					
					if(currentMode==1)
					{
						new Thread(new PushToJNIRunnable()).start();
					}

					//Force the device to stay awake.
					PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
					wl = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
					wl.acquire();
					
					//Wait for it to start executing.
					try {
						Thread.sleep(100);
					} catch (InterruptedException e) {
						Log.e(TAG,"Error on sleep");
					}
					
					if(burstIsBusy)
					{
						//Give the burst capture ~5 seconds.
						try {
							Thread.sleep(5000);
						} catch (InterruptedException e) {
							Log.e(TAG,"Error on sleep");
						}
						
						//Should definitely be done by now.
						burstIsBusy=false;
					}
				}
				
				else if(cameraLock==true&&cam!=null)
				{
					//Wait for it to become available.
					while(cameraLock)
					{
						try {
							Thread.sleep(20);
						} catch (InterruptedException e) {
							Log.e(TAG,"Error on sleep");
						}
					}
					
					//Lock the camera.
					cameraLock=true;
					
				}
				
				//See if I can do this from this thread.
				//willDoCameraBurst=true; //This works.
				
				if(debugMode) Log.i(TAG, "Finished runnable loop.");				

			}	
			//This is so much cleaner than the CountDownTimer.
			
			try {
				//TODO RC wants to know why this is here? what purpsose does it solve?
				//Sleep for to seconds.
				Thread.sleep(10000);
			} catch (InterruptedException e) {
			}
		}
			
		}
	}
	
	
	/**
	 *  Thread runnable to push data to JNI.
	 */
	//This should not only push to JNI, 
	//but it should do so efficiently and without many crashes.
	public class PushToJNIRunnable implements Runnable {
		public void run() {
			this.doPushing();
		}

		public void doPushing() {

			if (debugMode) {
				Log.i(TAG, "Pushing to JNI thread.");
				// This will perform the actual push work.
				// File sdDir = new File(EXT + "/" + DIR);
				// willDoCameraBurst=true;
				mDebug.logMessage(TAG, "Inside AsyncTask PushToJNI");
			}

			if (DEATH) {
				if (debugMode)
					mDebug.logMessage(TAG, "Death was called.");
				// break;
				return;
			}

			else {
				for (int i = 0; i < REQUESTLISTLENGTH; i++) {
					File testFile = new File(directory.getAbsolutePath() + "/"
							+ requestList[i]);
					if(debugMode) Log.i(TAG, "Testing file: \""+requestList[i]+"\"");
					
					if (testFile.isDirectory() ||  !testFile.exists() || testFile.length() == 0) {
						// Force stop the method if the file in question doesn't
						// actually exist.
						return;
					}
				}

				// Now that we know these files actually exist!
				File file1 = new File(directory.getAbsolutePath() + "/"
						+ requestList[0]);
				File file2 = new File(directory.getAbsolutePath() + "/"
						+ requestList[1]);

				String file1RawName = file1.getName().substring(0,
						file1.getName().lastIndexOf("."));
				String file2RawName = file2.getName().substring(0,
						file2.getName().lastIndexOf("."));

				int n = 0;

				// Always true.
				if (useNewProcess || !useNewProcess) {
					// This is the new new new process!
					// These functions calculate the area, not the width and
					// height.
					if (debugMode)
						Log.i(TAG,
								"Pushing to JNI using the new image pushing process...\n");

				//	perspective.setMinArea(minObjWidth * minObjHeight);
				//	perspective.setMaxArea(maxObjHeight * maxObjWidth);
				//	perspective.setCamHeight(camHeightAboveGround);
				//	perspective.setCamPitch(currentPitch);
					n = doRevisedJNI(file1RawName, null, file1
							.getPath(), cam.getParameters()
							.getVerticalViewAngle(), cam.getParameters()
							.getHorizontalViewAngle(), cam.getParameters()
							.getPictureSize().width, cam.getParameters()
							.getPictureSize().height);

				}

				else {
					// TODO put this back maybe?
					// n=l.oldPushImage(file1RawName, null, file1.getPath());
				}

				// Check for capture condition.
				if (n > 50) {
					// TODO put this back if it makes sense to later
					// Capture 10 seconds of video when the JNI code found
					// something interesting.
					// captureVideo(10000);
					// burstNum=5;
					// willDoCameraBurst=true;
				}

				if (debugMode)
					Log.i(TAG, "JNI Returned: " + n);
			}

		}
	}
	
	/**
	 * This will put a photo into a log during the image capture only mode so that the process only mode will be able to process through it
	 */
	public void logPhoto(String filename)
	{
		//Check to see whether or not we're allowed to log the filename.
		try {
			FileWriter logger=new FileWriter(processingQueueLog);
			
			//Insert a newline so that a future Scanner object will tokenize the filenames.
			//Toast.makeText(MainActivity.this, "Logging: "+filename, Toast.LENGTH_SHORT).show();
			if(processingQueueLog.exists())
			{
				//logger.append("/Removable/MicroSD/DCIM/.Processed/"+filename+"\n");
				logger.append(filename+"\n");
			}
			
			else
			{
				//logger.write("/Removable/MicroSD/DCIM/.Processed/"+filename+"\n");
				logger.write(filename+"\n");
			}
			
			logger.close();
		} catch (IOException e) {
			Log.e(TAG, "Error! Could not write to photo list!");
		}

	}
	

	private Camera getCameraInstance() {
		Camera c = null;

		try {
			c = Camera.open(CAM_3D);
		} catch (Exception e) {

		}
		if(c==null && debugMode) Log.i(TAG,"getCameraInstance FAILED");
		
		return c;
	}

	public String getRootSDCard() {

		String rootSDCardDir = "";
		try {
			// Open the file
			FileInputStream fs = new FileInputStream("/proc/mounts");

			DataInputStream in = new DataInputStream(fs);
			BufferedReader br = new BufferedReader(new InputStreamReader(in));

			String strLine;
			// StringBuilder sb = new StringBuilder();

			// Read File Line By Line
			while ((strLine = br.readLine()) != null) {

				// use the special android sdcard volume daemon(vold)
				// which is stupid, but it's how android mounts actual SD Cards
				// could also search for "fat" instead of vold.
				if (strLine.contains("vold")) {
					//if (debugMode)
					//	Log.i(TAG, "proc found: " + strLine);
					String parts[] = strLine.split(" ");
					//if (debugMode)
					//	Log.i(TAG, "Found SDCard Directory: " + parts[1]);
					rootSDCardDir = parts[1];
				}
			}

			// Close the stream
			in.close();
		} catch (Exception e) {
			// Catch exception if any
			Log.e(TAG,"COULD NOT SET SDCARD LOCATION!!!");
			e.printStackTrace();
		}

		// if we failed, use the env stuff. Will usually return the internal
		// writable
		// media and not the sd card. STUPID ANDROID!
		if (rootSDCardDir == "")
			return Environment.getExternalStorageDirectory().getPath();

		return rootSDCardDir;
	}

	private boolean loadPrefs() {

		String myRoot = getRootSDCard();
		Log.i(TAG,"Root SDCardDir: "+ myRoot);

		String key = this.getResources().getString(R.string.gallery_key);
		String myDir = myRoot
				+ "/"
				+ settings
						.getString(
								key,
								this.getResources().getString(
										R.string.gallery_default));

		if (debugMode)
			Log.i(TAG, "myDir: " + myDir);

		directory = new File(myDir);
		burstCaptureDir = new File(myRoot
				+ "/"
				+ settings.getString(key,
						this.getResources().getString(R.string.gallery_burst)));

		if (!burstCaptureDir.exists()) {
			burstCaptureDir.mkdir();
		}

		directory.mkdir();

		createFolder(R.string.chips_key, R.string.chips_default);
		createFolder(R.string.processed_key, R.string.processed_default);

		key = this.getResources().getString(R.string.applog_key);
		LOG_FILE = myRoot +"/"+ settings.getString(key, this.getResources().getString(R.string.applog_default));

		
		key = this.getResources().getString(R.string.dID_key);
		DEVICE_ID = settings.getString(key,
				this.getResources().getString(R.string.dID_default));

		key = this.getResources().getString(R.string.start_key);
		START_HOUR = settings.getInt("theHour" + key, 6);
		START_MINUTE = settings.getInt("theMinute" + key, 0);

		key = this.getResources().getString(R.string.stop_key);
		END_HOUR = settings.getInt("theHour" + key, 18);
		END_MINUTE = settings.getInt("theMinute" + key, 0);

		String value;

		/*
		 * key = this.getResources().getString(R.string.delete_key); value =
		 * this.getResources().getString(R.string.delete_default); DELETE_IMAGES
		 * = Boolean.valueOf(settings.getBoolean(key, Boolean.valueOf(value)));
		 */

		key = this.getResources().getString(R.string.skip_key);
		value = this.getResources().getString(R.string.skip_default);
		SKIP_JNI = Boolean.valueOf(settings.getBoolean(key,
				Boolean.valueOf(value)));

		prefsLoaded = true;

		File test = new File(getRootSDCard());
		if (!test.exists()) {
			Toast.makeText(this, "Please insert SD Card!!", Toast.LENGTH_LONG)
					.show();
			this.setTitle(getResources().getString(R.string.app_name)
					+ " - Insert SDCARD!");
			LOG_FILE = Environment.getExternalStorageDirectory()
					.getAbsolutePath() + "/SecuricsLogFile.txt";
		} else {
			this.setTitle(getResources().getString(R.string.app_name));
		}

		createFolder(R.string.gallery_key, R.string.gallery_default);
		createFolder(R.string.chips_key, R.string.chips_default);
		createFolder(R.string.processed_key, R.string.processed_default);
		createFolder(R.string.posExemplars_key, R.string.posExemplars_default);
		
		return true;
	}

	
	/*
	 * createFolder id_key: This is the R.string.KEY_HERE value of the key
	 * id_default: This is the folders name. Will check for an ACTUAL SDCard. If
	 * none exists will use
	 * Environment.getExternalStorageDirectory()/R.string.DEFAULT_HERE
	 */
	private void createFolder(int id_key, int id_default) {

		String key = this.getResources().getString(id_key);
		String value = getRootSDCard() + "/"
				+ settings.getString(key, getResources().getString(id_default));

		File test = new File(value);
		if (!test.exists()) {
			if (mDebug != null)
				mDebug.logMessage(TAG, "Creating folder: " + value);
			else if (debugMode)
				Log.i(TAG, "Creating folder: " + value);
			test.mkdirs();
		} else {
			if (mDebug != null)
				mDebug.logMessage(TAG, value + " already exists!");
			else if (debugMode)
				Log.i(TAG, value + " already exists!");
		}
		test = new File(value + "/.nomedia");
		try {
			test.createNewFile();
		} catch (IOException e) {
			if (mDebug != null)
				mDebug.logMessage(TAG, "Failed to create " + value
						+ "/.nomedia");
			else if (debugMode)
				Log.i(TAG, "Failed to create " + value + "/.nomedia");
			e.printStackTrace();
		}

		if (mDebug != null && debugMode)
			mDebug.logMessage(TAG, "Finished with " + value);
		else if (debugMode)
			Log.i(TAG, "Finished with " + value);

	}

	private void saveCameraParams() {
		Camera.Parameters cp = cam.getParameters();

		if (debugMode)
			Log.i(TAG, "saveCameraParams()");

		List<String> modes;

		modes = cp.getSupportedFocusModes();
		Set<String> set = new HashSet<String>(modes);

		if (settings == null)
			settings = PreferenceManager
					.getDefaultSharedPreferences(getApplicationContext());

		Editor editor = settings.edit();
		try {
			editor.putStringSet("CamPrefsFocus", set);
		} catch (Exception e) {
			e.printStackTrace();
		}

		modes = cp.getSupportedSceneModes();

		// This is one of the two lines that causes the emulator to crash.
		// Revision 1 Michael Gohde
		if (modes != null) {
			set = new HashSet<String>(modes);
		}

		// End of line that causes the emulator to crash.

		try {
			editor.putStringSet("CamPrefsScene", set);
		} catch (Exception e) {
			// e.printStackTrace();
		}

		modes = cp.getSupportedWhiteBalance();
		set = new HashSet<String>(modes);
		try {
			editor.putStringSet("CamPrefsWBalance", set);
		} catch (Exception e) {
			// e.printStackTrace();
		}

		editor.commit();

		if (debugMode)
			Log.i(TAG, "Done saving params");
	}

	private void showPopupMenu(View v) {
		PopupMenu popupMenu = new PopupMenu(MainActivity.this, v);
		popupMenu.getMenuInflater()
				.inflate(R.menu.ss_main, popupMenu.getMenu());

		// This is for adding an indicator of remaining space!!!
		popupMenu.getMenu().add(spaceAvailable() + " MB Available");

		// Important! Need to give the proper config to the C code
		// This needs to happen AFTER onResume (due to opencv/library load bug),
		// but before jni is really used. This seems a good a place as any!
		if (!gaveConfig) {
			gaveConfig = sendConfigToJNI();
		}

		if (RUNNING) {
			popupMenu.getMenu().removeGroup(R.id.startGroup);
		} else {
			popupMenu.getMenu().removeGroup(R.id.stopGroup);
		}

		// I see.
		popupMenu.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
			@Override
			public boolean onMenuItemClick(MenuItem item) {

				File test = new File(getRootSDCard());
				if (!test.exists()) {
					Toast.makeText(MainActivity.this,
					"Please insert SDCard!!", Toast.LENGTH_LONG).show();
					LOG_FILE = Environment
						.getExternalStorageDirectory()
						.getAbsolutePath()
						+ "SecuricsLogFile.txt";
				} else {
					Toast.makeText(MainActivity.this, item.toString(),
					Toast.LENGTH_LONG).show();
				}
				
				//Since this code is big and now a duplicate of when the menu item is pressed
				// it's been centralized into one function.
				if (item.toString().equalsIgnoreCase("start")) {
					startStop();

				} else if (item.toString().equalsIgnoreCase("stop")) {
					startStop();
				}

				return true;
			}
		});
		popupMenu.show();
	}

	private long spaceAvailable() {

		File sdcard = new File(getRootSDCard());
		if (!sdcard.exists()) {
			Toast.makeText(this, "Please insert SDCard!!", Toast.LENGTH_SHORT)
					.show();
			if (mDebug != null && debugMode)
				mDebug.logMessage(TAG, "NO sdCard found. Exiting!");
			else if (debugMode)
				Log.e(TAG, "NO sdCard found. Exiting");
			this.finish();
		}

		long spaceInBytes = directory.getUsableSpace();

		long spaceInMB = spaceInBytes / (1024 * 1024);

		if (debugMode)
			Log.i(TAG, "spaceAvailable: " + spaceInMB + "MB");

		return spaceInMB;

	}

	private boolean sendConfigToJNI() {

		if (mDebug != null && debugMode)
			mDebug.logMessage(TAG, "sendConfigToJNI()");
		else if(debugMode)
			Log.i(TAG, "sendConfigToJNI()");

		String myDir = getRootSDCard() + "/";
		
		String params[];
		int response = 0;
		
		// General Squirrel Parameters
		params = getResources().getStringArray(R.array.General_Squirrel_Params);
		for (int i = 0; i < params.length; i += 2) {
			String p = params[i];
			String d = params[i + 1];
			String v = settings.getString(p, d);
			response += giveConfig(p, v);
		}
		// File and Folder locations
		String myKey, myValue;
		myKey = getResources().getString(R.string.processlog_key);
		myValue = myDir + settings.getString(myKey,
				getResources().getString(R.string.processlog_default));
		response += giveConfig(myKey, myValue);
		myKey = getResources().getString(R.string.output_key);
		myValue = myDir + settings.getString(myKey,
				getResources().getString(R.string.output_default));
		response += giveConfig(myKey, myValue);
		myKey = getResources().getString(R.string.dID_key);
		myValue = settings.getString(myKey,
				getResources().getString(R.string.dID_default));
		response += giveConfig(myKey, myValue);
		myKey = getResources().getString(R.string.gallery_key);
		myValue = myDir + settings.getString(myKey,
				getResources().getString(R.string.gallery_default));
		response += giveConfig(myKey, myValue);
		myKey = getResources().getString(R.string.posExemplars_key);
		myValue = myDir + settings.getString(myKey,
				getResources().getString(R.string.posExemplars_default));
		response += giveConfig(myKey, myValue);
		myKey = getResources().getString(R.string.processed_key);
		myValue = myDir + settings.getString(myKey,
				getResources().getString(R.string.processed_default));
		response += giveConfig(myKey, myValue);
		myKey = getResources().getString(R.string.chips_key);
		myValue = myDir + settings.getString(myKey,
				getResources().getString(R.string.chips_default));
		response += giveConfig(myKey, myValue);
		myKey = getResources().getString(R.string.model_key);
		myValue = myDir + settings.getString(myKey,
				getResources().getString(R.string.model_default));
		response += giveConfig(myKey, myValue);
		myKey = getResources().getString(R.string.modelName_key);
		myValue = settings.getString(myKey, getResources().getString(R.string.modelName_default));
		response += giveConfig(myKey, myValue);
		myKey = getResources().getString(R.string.negExemplar_key);
		myValue = myDir + settings.getString(myKey,
				getResources().getString(R.string.negExemplar_default));
		response += giveConfig(myKey, myValue);
		myKey = getResources().getString(R.string.posExemplarMax_key);
		myValue = myDir + settings.getString(myKey,
				getResources().getString(R.string.posExemplarMax_default));
		response += giveConfig(myKey, myValue);
		myKey = getResources().getString(R.string.posExemplarMin_key);
		myValue = myDir + settings.getString(myKey,
				getResources().getString(R.string.posExemplarMin_default));
		response += giveConfig(myKey, myValue);

		
		if (response != 0 && debugMode)
			Log.i(TAG, "sendConfig value = " + response);
		if (response == 0)
			return true;
		return false;

	}


	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		
	}

	@Override
	public void onSensorChanged(SensorEvent event) {
		//This is the only value I'm interested in here.
		currentPitch = event.values[1];
		
		//This happens far too often to log.
		//System.err.println("Current pitch="+currentPitch);
		
	}

	private Camera.AutoFocusCallback FocusCallback = new Camera.AutoFocusCallback() {

		@Override
		public void onAutoFocus(boolean success, Camera camera) {
			//cam.takePicture(null, null, PicCallback);
			
			cameraLock=true;
			cam.takePicture(null, null, PicCallback);
			cameraLock=false;
		}
	};

	private PictureCallback PicCallback = new PictureCallback() {

		@Override
		public void onPictureTaken(byte[] data, Camera camera) {
			cam.startPreview();
			boolean haveSpace = false;
			while (!haveSpace) {
				if (isSpaceAvailable(data.length * 10)) {
					new SavePhotoTask().execute(data);
					spaceAvailable = true;
					haveSpace = true;
				}
			}
			
			//Unlock the camera for reuse after this.
			cameraLock=false;
		}
	};

	private boolean isSpaceAvailable(long length) {
		boolean isAvailable = true;
		long spaceInBytes = directory.getUsableSpace();
		if (length > spaceInBytes) {
			isAvailable = false;
		}

		String spaceAvailable = spaceInBytes / (1024 * 1024) + " MB available";
		if (isAvailable)
			spaceAvailable += " is enough space.";
		else
			spaceAvailable += "is NOT enough space!";

		if(debugMode)
		{
			//mDebug.logMessage(TAG, spaceAvailable);
			mDebug.logHeap(this.getClass());
		}
		return isAvailable;
	}

	private class SavePhotoTask extends AsyncTask<byte[], Void, Void> {

		@Override
		protected Void doInBackground(byte[]... args) {
			long startTime=System.currentTimeMillis();
			
			SimpleDateFormat sdf = new SimpleDateFormat("MM-dd-yy_HH.mm.ss",
					Locale.US);
			String dateString = sdf
					.format(new Date(System.currentTimeMillis()));
			
			if(debugMode) Log.d(TAG, dateString);
			
			//Set the current request to this value.
			requestList[currentRequest]=DEVICE_ID + "_" + dateString + System.currentTimeMillis()+".jpeg";
			//Write the data to the log.
			logPhoto(requestList[currentRequest]);
			
			//This should be an easy way of checking whether or not this was called from burst capture.

			File picFile = new File(directory, requestList[currentRequest]);				

			try {
				FileOutputStream fos = new FileOutputStream(picFile);
				fos.write(args[0]);
				fos.close();
			} catch (FileNotFoundException e) {
				if(debugMode) Log.e(TAG, "File not found: " + e.getMessage());
			} catch (IOException e) {
				if(debugMode) Log.e(TAG, "Error accessing file: " + e.getMessage());
			}

			if(debugMode) mDebug.logMessage(TAG, "Saving file: " + picFile.getName());
			
			//Move on to the next one.
			currentRequest++;
			
			if(currentRequest>=REQUESTLISTLENGTH)
			{
				currentRequest=0;
			}
			
			long endTime=System.currentTimeMillis();
			
			if(debugMode) System.err.println("SavePhotoTask took: "+(endTime-startTime));

			/*
			 * RC Commented this out. It doesn't really work.
			//These are the settings I used for the test images.
			perspective.setDegPointedAtGround(currentPitch);
			perspective.setDistAboveGround(camHeightAboveGround);
			//This is close to my phone camera's 40.74.
			//Gets the vertical view angle of the camera.
			//perspective.setVerticalDegRangeOfCamera(cam.getParameters().getVerticalViewAngle());
			perspective.setVerticalDegRangeOfCamera(cameraVerticalDegreeRange);
			perspective.runTransform(directory.getAbsolutePath()+"/"+picFile.getName(), directory.getAbsolutePath()+"/"+"CORRECTED-"+picFile.getName());
			
			//Change the request so that JNI works on the corrected image.
			requestList[currentRequest]=directory.getAbsolutePath()+"/"+"CORRECTED-"+picFile.getName();
			*/
			return null;
		}
	}
	

	/**
	 * This function runs a resolution dialog.
	 */
	private void runResolutionDialog(Camera.Parameters params)
	{
		
		 
		//double numImgsAtRes;
		//View tempView=new View();
		//This value represents the first image in the list that doesn't fit the RAM/pixel ratio.
		//
		int badPos=-1;
		outDialog=new Dialog(MainActivity.this);
		Display disp=getWindowManager().getDefaultDisplay();
		String selectedObject=null;
		ListView modeList=new ListView(this);
		
		outDialog.setTitle("Change Resolution.");
		AlertDialog.Builder builder=new AlertDialog.Builder(MainActivity.this);
		//Declare the new list of strings.
		resStringList=new String[params.getSupportedPictureSizes().size()];
		//Loop through the list.
		for(int i=0;i<params.getSupportedPictureSizes().size();i++)
		{
			Log.i(TAG, ""+params.getSupportedPictureSizes().get(i));
			
			resStringList[i]=params.getSupportedPictureSizes().get(i).width+"x"+params.getSupportedPictureSizes().get(i).height;
			
			if(!calcForRAM(params.getSupportedPictureSizes().get(i).width, params.getSupportedPictureSizes().get(i).height))
			{
				//This is a crude but effective way of letting the user know whether or not it is possible to use
				//the selected resolution.
				resStringList[i]+=" X ";
			}
			
			if(params.getSupportedPictureSizes().get(i).equals(params.getPictureSize()))
			{
				resStringList[i]+=" -- Current Resolution";
			}
			
			resStringList[i]+="   Estimated pictures: "+
					calcNumOfPictures(params.getSupportedPictureSizes().get(i).width, params.getSupportedPictureSizes().get(i).height);
			//calcDays(params);
			resStringList[i]+=". This should last for "+Math.round(calcDays(params, i))+" days.";
		}
		
		ArrayAdapter<String> stringListAdapter=new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, resStringList);
		//So far so good.
		modeList.setAdapter(stringListAdapter);
		//modeList.getChildAt(1).setAlpha(alpha)
		//Allow the view to be clicked.
		modeList.setClickable(true);
		modeList.setOnItemClickListener(new OnItemClickListener()
		{

			@Override
			public void onItemClick(AdapterView<?> parentObj, View viewIn, int pos,
					long id) 
			{
				setRes(pos);
				
				outDialog.cancel();
			}
		
		});
		
		builder.setView(modeList);
		outDialog=builder.create();
		outDialog.show();

	}
	
	public double calcDays(Camera.Parameters params, int idNum)
	{
		double days;
		int picNum;
		
		picNum=calcNumOfPictures(params.getSupportedPictureSizes().get(idNum).width, params.getSupportedPictureSizes().get(idNum).height);
		
		//60 captures per hour over 24 hours. This is not simplified to allow for easier understanding of the function.
		days=picNum/60/24.0;
		
		return days;
	}
	
	/**This function exists to automatically determine which camera resolution is best for the user given the amount
	 * of RAM available in the device.
	 */
	public void fitRAM()
	{
		/**
		 * Open up a kernel data structure for reading:
		 */
		File procMeminfo=new File("/proc/meminfo");
		try {
			Scanner procReader=new Scanner(procMeminfo);
			//The first line of /proc/meminfo shows the total amount of RAM available to the system in kilobytes.
			String totalMemLine=procReader.nextLine();
			
			if(debugMode)
			{
				Log.i(TAG, totalMemLine);
				//Toast.makeText(MainActivity.this, totalMemLine, Toast.LENGTH_LONG).show();
			}
			//Reset the scanner so that we can re-read from the beginning.
			procReader=new Scanner(procMeminfo);
			
			//Discard the leading text.
			procReader.next();
			long kBytes=procReader.nextLong();
			//Toast.makeText(MainActivity.this, "Read value: "+kBytes, Toast.LENGTH_LONG).show();
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		/*
		long ramTotal;
		ActivityManager.MemoryInfo memInfo=new ActivityManager.MemoryInfo();
		ramTotal=memInfo.totalMem;
		ramTotal=
		Log.i(TAG, "Total System Memory: "+ramTotal);
		//Display a toast so that the memory data doesn't get caught by all of the clutter:
		Toast.makeText(MainActivity.this, "Total System Memory: "+ramTotal, Toast.LENGTH_LONG).show();*/
	}
	
	/**This function will accept a resolution value and return whether or not that selected resolution should work with the device while processing.
	 * Due to the variable nature of image compression and memory configurations, this will be calculated as a proportion from the baseline established
	 * on the Asus testing tablet.
	 */
	public boolean calcForRAM(long length, long width)
	{
		//This allows other code to use any filtered resolutions if the file read fails.
		boolean outBool=true;
		//This value represents the ratio between total RAM and pixels. 
		double baseline=998696/7990272.0;
		
		try {
			File procMeminfo=new File("/proc/meminfo");
			Scanner procReader=new Scanner(procMeminfo);
			procReader.next();
			long kBytes=procReader.nextLong();
			
			//In this case, the image resolution is either too large, or the amount of RAM is too small.
			if(baseline>((double)kBytes/(length*width)))
			{
				outBool=false;
			}
		} catch (FileNotFoundException e) {
			Log.e(TAG,"/proc/meminfo not found");
			e.printStackTrace();
		}
		
		
		return outBool;
	}

	/**
	 * Set the picture resolution given the position in the array this was in.
	 */
	private void setRes(int pos)
	{
		//error
		//publicWidth=Integer.parseInt(res);
		//publicHeight=Integer.parseInt(res);
		publicWidth=cam.getParameters().getSupportedPictureSizes().get(pos).width;
		publicHeight=cam.getParameters().getSupportedPictureSizes().get(pos).height;
		
		if(publicWidth>0&&publicHeight>0)
		{
			//cam.stopPreview();
			Camera.Parameters tempParams;
			tempParams=camPreview.cam.getParameters();
			//This should set the preview so that it is consistent with the selected resolution.
			//tempParams.setPreviewSize(publicWidth, publicHeight);
			tempParams.setPictureSize(publicWidth, publicHeight);
			//tempParams.setPictureSize(1024, 768);
			
			int maxPreviewWidth, maxPreviewHeight, minPreviewWidth, minPreviewHeight, listLength;
			listLength=tempParams.getSupportedPreviewSizes().size();
			
			//Get the maximum and minimum supported preview resolutions.
			maxPreviewWidth=tempParams.getSupportedPreviewSizes().get(listLength-1).width;
			maxPreviewHeight=tempParams.getSupportedPreviewSizes().get(listLength-1).height;
			minPreviewWidth=tempParams.getSupportedPreviewSizes().get(0).width;
			minPreviewHeight=tempParams.getSupportedPreviewSizes().get(0).height;
			
			//Ensure that the preview will actually fit.
			if(publicWidth>maxPreviewWidth||publicHeight>maxPreviewHeight)
			{
				tempParams.setPreviewSize(maxPreviewWidth, maxPreviewHeight);
			}
			
			else if(publicWidth<minPreviewWidth||publicHeight<minPreviewHeight)
			{
				tempParams.setPreviewSize(minPreviewWidth, minPreviewHeight);
			}
			
			//Set it to the selected resolution.
			else
			{
				tempParams.setPreviewSize(publicWidth, publicHeight);
			}
			//cam.setParameters(cam.getParameters().setPictureSize(publicWidth, publicHeight));
			camPreview.cam.setParameters(tempParams);
			//cam.startPreview();
			
			if(calcNumOfPictures(publicWidth, publicHeight)<500)
			{
				//This should warn the user about the potential for filling the device's storage very quickly. 
				Toast.makeText(MainActivity.this, "WARNING! The selected resolution may fill your devices's storage very quickly!", 5).show();
			}
		}
	}
	
	/**This function accepts a resolution and estimates the number of images at this resolution
	 * that will fit into the device's storage. Since JPEG is a compressed format, this is more of a guess
	 * than a real estimate.
	 */
	public int calcNumOfPictures(int width, int height)
	{
		//An 8 megapixel image is roughly 2 MB:
		double base=2.0;
		int numOfPictures;
		double spaceFree;
		double picSize=(base*((width*height)/7990272.0));
		//Log.i(TAG, "")
		
		//Get number of megabytes free:
		spaceFree=directory.getUsableSpace()/1024/1024;
		
		numOfPictures=Math.round((float)(spaceFree/picSize));
		
		if(debugMode)
		{
			Log.i(TAG, "picture size @ "+width+", "+height+" = "+picSize);
			Log.i(TAG, "spaceFree/picSize="+spaceFree/picSize);
		}
		return numOfPictures;
	}
	
	/**This dialog allows the user to enter maximum areas for detected
	 * objects.
	 */
	private void runMaxLengthWidthDialog()
	{
		final LinearLayout dialogLayout=new LinearLayout(this.getBaseContext());
		dialogLayout.setOrientation(LinearLayout.VERTICAL);
		AlertDialog.Builder builder=new AlertDialog.Builder(MainActivity.this);
		builder.setTitle("Set Maximum Length and Width");
		builder.setMessage("Please enter the maximum detectable height and width. " +
				"These will be converted to an area.");
		final EditText widthInput=new EditText(this);
		final EditText heightInput=new EditText(this);
		widthInput.setText(""+maxObjWidth);
		heightInput.setText(""+maxObjHeight);
		//Hopefully these values are being pushed into the end of the list.
		dialogLayout.addView(widthInput);
		dialogLayout.addView(heightInput);
		builder.setView(dialogLayout);
		//builder.set
		
		builder.setPositiveButton("Enter", new DialogInterface.OnClickListener() 
		{
			
			@Override
			public void onClick(DialogInterface dialog, int which) 
			{
				String in1=widthInput.getText().toString();
				String in2=heightInput.getText().toString();
				try{
				maxObjWidth=Double.parseDouble(in1);
				maxObjHeight=Double.parseDouble(in2);
				}catch(Exception e) {
					mDebug.logMessage(TAG, "Number format: "+in1+ " or "+in2);
				}
				outDialog.dismiss();
			}
		});
		
		builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() 
		{
			
			@Override
			public void onClick(DialogInterface dialog, int which) 
			{
				outDialog.dismiss();
			}
		});
		
		outDialog=builder.create();
		outDialog.show();
	}
	
	private void runMinLengthWidthDialog()
	{
		final LinearLayout dialogLayout=new LinearLayout(this.getBaseContext());
		dialogLayout.setOrientation(LinearLayout.VERTICAL);
		AlertDialog.Builder builder=new AlertDialog.Builder(MainActivity.this);
		builder.setTitle("Set Minimum Length and Width");
		builder.setMessage("Please enter the minimum detectable height and width. " +
				"These will be converted to an area.");
		final EditText widthInput=new EditText(this);
		final EditText heightInput=new EditText(this);
		widthInput.setText(""+minObjWidth);
		heightInput.setText(""+minObjHeight);
		//Hopefully these values are being pushed into the end of the list.
		dialogLayout.addView(widthInput);
		dialogLayout.addView(heightInput);
		builder.setView(dialogLayout);
		//builder.set
		
		builder.setPositiveButton("Enter", new DialogInterface.OnClickListener() 
		{
			
			@Override
			public void onClick(DialogInterface dialog, int which) 
			{
				String in1=widthInput.getText().toString();
				String in2=heightInput.getText().toString();
				
				try{
				//There we are.
				minObjWidth=Double.parseDouble(in1);
				minObjHeight=Double.parseDouble(in2);
				}catch(Exception e) {
					mDebug.logMessage(TAG, "Number format: "+in1+ " or "+in2);
				}
				outDialog.dismiss();
			}
		});
		
		builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() 
		{
			
			@Override
			public void onClick(DialogInterface dialog, int which) 
			{
				outDialog.dismiss();
			}
		});
		
		outDialog=builder.create();
		outDialog.show();
	}

	/**
	 * This is for the vertical distance above the ground dialog box.
	 */
	private void runVertDialog()
	{
		//outDialog=
		//Create a dialog box builder with context.
		AlertDialog.Builder builder=new AlertDialog.Builder(MainActivity.this);
		builder.setTitle("Set Distance Above Ground");
		builder.setMessage("Please enter how high the camera is off the ground.");
		final EditText dataInput=new EditText(this);
		//Set this to the current vert dist for context.
		dataInput.setText(""+camHeightAboveGround);
		builder.setView(dataInput);
		
		//Eclipse really is awesome, since it just generated this whole block!
		builder.setPositiveButton("Enter", new DialogInterface.OnClickListener() 
		{
			
			@Override
			public void onClick(DialogInterface dialog, int which) 
			{
				try {
				double internalVertDist=Double.parseDouble(dataInput.getText().toString());
				//Set the variable.
				camHeightAboveGround=internalVertDist;
				}catch(Exception e) {
					mDebug.logMessage(TAG, "Number format: "+dataInput.getText().toString());
				}
				outDialog.dismiss();
			}
		});
		
		builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() 
		{
			
			@Override
			public void onClick(DialogInterface dialog, int which) 
			{
				//Dismiss the dialog. The user did nothing.
				outDialog.dismiss();
			}
		});
		
		outDialog=builder.create();
		outDialog.show();
	}

	
	
	private void changeFreqDialog()
	{
		/*
		
		//This should launch the frequency change activity.
		Intent freqChangeIntent = new Intent(this,
				FreqChanger.class);
		freqChangeIntent.putExtra("curtimermax", timeInterval);
		freqChangeIntent.putExtra("curtimeincr", numOfPicsPerCapture);
		MainActivity.this.startActivityForResult(freqChangeIntent, 1);
		
		//The result should be packed in this activity's intent.
		
		//getIntent().getLongExtra("result", ctrMax);
*/
		
		
		final LinearLayout dialogLayout=new LinearLayout(this.getBaseContext());
		dialogLayout.setOrientation(LinearLayout.VERTICAL);
		AlertDialog.Builder builder=new AlertDialog.Builder(MainActivity.this);
		builder.setTitle("Frequency Changer");
		//builder.setMessage("Please enter the minimum detectable height and width. " +
		//		"These will be converted to an area.");
		final TextView currCapFreq =     new TextView(this);
		final TextView newCapFreq =      new TextView(this);
		final TextView newCapPerClock =  new TextView(this);
		
		final TextView currTimesEvery =  new TextView(this);
		final EditText capFreq =         new EditText(this);
		final EditText capPerClock =     new EditText(this);
		
		currCapFreq.setText(R.string.freq_currentCapFreq);
		currTimesEvery.setText(numOfPicsPerCapture + " times every "+timeInterval+" ms");
		newCapFreq.setText(R.string.freq_newCapFreq);
		capFreq.setText(""+numOfPicsPerCapture);
		newCapPerClock.setText(R.string.freq_newCapPerClock);
		capPerClock.setText(""+timeInterval);
		
		dialogLayout.addView(currCapFreq);
		dialogLayout.addView(currTimesEvery);
		dialogLayout.addView(newCapFreq);
		dialogLayout.addView(capFreq);
		dialogLayout.addView(newCapPerClock);
		dialogLayout.addView(capPerClock);
		builder.setView(dialogLayout);
		//builder.set
		
		builder.setPositiveButton("Enter", new DialogInterface.OnClickListener() 
		{
			
			@Override
			public void onClick(DialogInterface dialog, int which) 
			{
				try {
				timeInterval = Long.parseLong(capPerClock.getText().toString());
				numOfPicsPerCapture = Integer.parseInt(capFreq.getText().toString());
				}catch(Exception e) {
					mDebug.logMessage(TAG, "Number format: "+capPerClock.getText().toString()+ 
							" or " + capFreq.getText().toString());
				}
				
				outDialog.dismiss();
			}
		});
		
		builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() 
		{
			
			@Override
			public void onClick(DialogInterface dialog, int which) 
			{
				outDialog.dismiss();
			}
		});
		
		outDialog=builder.create();
		outDialog.show();
	}

	
	
	/**
	 * Capture a brief bit of video when PushToJNI says so:
	 * NOTE: This funtion does not cause a crash in the program.
	 */
	public void captureVideo(int lengthInMillis)
	{
		
		//Just to keep these variables in sight.
		//videoOutputURI=null;
		//int n=CAPTURE_VIDEO_REQUEST_CODE;
		//Set up the actual video capture intent.
		//Set up the video file name:
		
		//This is just an easy way of getting the highest possible resolution out of a camera. This is mostly for
		//compatibility with terrible devices.
		//using -1 because 0-indexed
		int vidResMaxSizePos=cam.getParameters().getSupportedVideoSizes().size();
		int maxVidHeight=cam.getParameters().getSupportedVideoSizes().get(vidResMaxSizePos-1).height;
		int maxVidWidth=cam.getParameters().getSupportedVideoSizes().get(vidResMaxSizePos-1).width;
		
		cam.stopPreview();
		//According to the google developer website, "Starting with Android 4.0(...), the Camera.lock...are managed for 
		//you automatically.
		cam.unlock();
	
		//Generate the video name.
		SimpleDateFormat sdf = new SimpleDateFormat("MM-dd-yy_HH.mm.ss",
				Locale.US);
		String dateString = sdf
				.format(new Date(System.currentTimeMillis()));
		if(debugMode) Log.d(TAG, dateString);
		//Set the current request to this value.
		String videoName=DEVICE_ID + "_" + dateString + System.currentTimeMillis()+".mp4";
		
		MediaRecorder videoInput=new MediaRecorder();
		videoInput.setCamera(cam);
		//Just go with the default
		//720p should be more than enough.
		
		videoInput.setAudioSource(MediaRecorder.AudioSource.MIC);
		videoInput.setVideoSource(MediaRecorder.VideoSource.CAMERA);

		//This code produces a video file that is about 1 megabyte. 
		
		//CamcorderProfile.get(CamcorderProfile.QUALITY_720P).
		//Set the video format and encoding.
		//videoInput.setProfile(CamcorderProfile.get(CamcorderProfile.QUALITY_720P));
		videoInput.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);
		//I think this should work.
		videoInput.setVideoEncoder(MediaRecorder.VideoEncoder.H264);
		videoInput.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_NB);
		//videoInput.setVideoSize(800, 600);
		videoInput.setVideoEncodingBitRate(1200000);
		//CamcorderProfile highSettings=CamcorderProfile.get(CamcorderProfile.QUALITY_720P);
		//This needs to be tweaked later on, since this resolution is apparently too much for the tablet.
		//videoInput.setVideoSize(720, 480);
		//Let's go with a value that everything should be able to use.
		videoInput.setVideoSize(maxVidWidth,maxVidHeight);
		//videoInput.setVideoSize()
		//videoInput.setVideoSize(800, 600);
		//videoInput.setProfile(highSettings);
		//Should give the program the video recording filename.
		videoInput.setOutputFile(directory.getAbsolutePath()+"/"+videoName);
		//30 seconds should be enough.
		videoInput.setMaxDuration(lengthInMillis);
		
		try {
			videoInput.prepare();
		} catch (IllegalStateException e1) {
			Log.e(TAG,"illegal state exception");
			//e1.printStackTrace();
		} catch (IOException e1) {
			Log.e(TAG,"ioexception()");
			//e1.printStackTrace();
		}
		
		videoInput.start();

		//Sleep for the amount of time necessary.
		try {
			Thread.sleep(lengthInMillis);
		} catch (InterruptedException e1) {
			e1.printStackTrace();
		}
		
		videoInput.stop();
		videoInput.reset();
		videoInput.release();
		
		try {
			cam.reconnect();
		} catch (IOException e) {
			//e.printStackTrace();
		}
		
		//cam.startPreview();*/
	}

	
	/**
	 * This is the main running part of the thread.
	 */
	public class BurstRunnable implements Runnable
	{
		public void run()
		{
			//boolean cameraLockTest=true;
			burstIsBusy=true;
			
			int i;
			i=0;
			//In this loop, the thread is going to check whether or not the AsyncTask is busy so it doesn't
			//get overwhelmed as seen with the for loop in the first attempt at doing this.
			while(burstCanRun)
			{

				if (DEATH) 
				{
					if (mDebug != null && debugMode)
						mDebug.logMessage(TAG, "Death was called.");
					else if(debugMode)
						Log.i(TAG, "Death was called");
					
					//this.isRunning=false;
				}


				//Time and date calculations omitted so that in the nearly impossible instance that
				//JNI detects something at the end of the capture time, pictures will be taken.

					//This will check whether or not the camera is usable and unlocked.
					if (cam != null&&cameraLock==false)
					{
						cam.stopPreview();
						//This should execute quickly enough for it to prevent another
						//process from getting in.
						cameraLock=true;
						//This was cam.autofocus or something like that.
						//We don't have time to focus, though, and it can be assumed
						//that if it is looking at the same thing, it need not re-focus.
						if(burstCanRun)
						{
							//camera is unlocked in the callback method.
							cam.takePicture(null, null, PicCallback);
						}
						
						cam.startPreview();
						
						cameraLock=false;
					}
					
					else
					{
						//Wait through the lock; unfortunately, there's a bit of a problem with 100% CPU use.
						while(cameraLock)
						{
							try {
								//Sleep for a little while. Hopefully, the lock will be removed by then.
								Thread.sleep(20);
							} catch (InterruptedException e) {
								// TODO Auto-generated catch block
								//e.printStackTrace();
							}
						}
						
						cameraLock=true;
						
						if(burstCanRun)
						{
							cam.takePicture(null, null, PicCallback);
						}
						
						cameraLock=false;
					}

				// isApplicationSentToBackground(getBaseContext());

				//this.isRunning=false;
				i++;
				
				//This will take 2 pictures in succession. 
				if(i>=burstNum)
				{
					burstCanRun=false;
					//break;
					
					if(debugMode) Log.i(TAG, "i>=5, stopping loop.");
					
					cam.startPreview();
					burstIsBusy=false;
					return;
				}


				
				try {
					Thread.sleep(100);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					//e.printStackTrace();
					//burstCanRun=false;
					//burstIsBusy=false;
					//return;
				}
				
				if(debugMode) Log.i(TAG, "Done with iteration "+i+" , starting iteration "+(i+1));
			}
			
			burstIsBusy=false;
			if(debugMode) Log.i(TAG, "outside loop.");
		}
	}
	
	public class BurstCheckerRunnable implements Runnable
	{

		@Override
		public void run() 
		{
			//As long as the program remains alive.
			while(!DEATH)
			{
				if(willDoCameraBurst)
				{
					try {
						captureBurst();
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						//e.printStackTrace();
					}
					
					willDoCameraBurst=false;
				}
				
				//So that it doesn't waste too many of its assigned CPU cycles:
				try {
					//Will tick twice for every tick in the CountDownTimer.
					Thread.sleep(20);
				} catch (InterruptedException e) {
					//e.printStackTrace();
				}
			}
		}
		
	}
	
	/**
	 * This is the camera burst capture function.
	 */
	//To prevent distortion and make this run at a reasonable rate, I will implement threads.
	//This has a disadvantage on single-core devices due to the overhead.
	private void captureBurst() throws InterruptedException
	{
		burstCanRun=false;
		if(burstCaptureThread!=null)
		{
			burstCaptureThread.interrupt();
		}
		//This will cause the garbage collector to remove the other running thread.
		burstCaptureThread=null;

		burstCaptureThread=new Thread(new BurstRunnable());
		
		//Start it up!
		burstCanRun=true;
		burstCaptureThread.start();
	}


	
	/**
	 * Thread runnable to control the autofocus when the program is running.
	 */
	public class AutoFocusRunnable implements Runnable
	{
		public void run()
		{
			//Will attempt to autofocus the camera for the duration of the program's existance.
			while(!DEATH)
			{
				
				/*
				if(cam!=null&&cameraLock==false)
				{
					
					//Make this happen every second or so.
					try {
						Thread.sleep(2000);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						//e.printStackTrace();
						
						//This should only happen when the program is stopping.
						return;
					}
					//Set to sleep for 2 seconds before trying to autofocus because the camera might not be ready yet.
					cameraLock=true;
					
					//Check again to be sure.
					if(cam!=null)
					{
						
						//cam.cancelAutoFocus();
						//This is obsoleted by FOCUS_MODE_CONTINUOUS_VIDEO
						//cam.autoFocus(null);
					}
					
					//Wait for the camera to actually do the autofocus.
					//try {
					//	Thread.sleep(500);
					//} catch (InterruptedException e) {
					//	//Nothing really needs to be done here. Carry on.
					//}
					
					cameraLock=false;
					
					//This is placed here since the autofocus is run in the tightest loop.
					
				}*/
				
				if(spaceAvailable()<10)
				{
					//If we're allowed to autoDelete.
					if(mayAutoDelete)
					{
						autoDelete();
					}
				}

			}
		}
	}
	
	/**
	 * This function automatically deletes the images that have been moved to the ".Processed" directory.
	 */
	private void autoDelete()
	{
		int i;
		long oldTime, newTime;
		//This represents the individual file to be deleted.
		File fileList[];
		File delDir;
		String dirLoc;
		//String fileList[];
		String key = this.getResources().getString(R.string.gallery_key);
		
		dirLoc=settings.getString(key, this.getResources().getString(R.string.processed_default));
		delDir=new File(dirLoc);
		
		//If the configuration is, in fact, pointing to a directory.
		if(delDir.isDirectory())
		{
			fileList=delDir.listFiles();
			
			//I still have to get used to these for-each loops.
			for(File temp:fileList)
			{
				//This could prevent any potential mishaps with the directories named "." and "..", or any directories, for that matter.
				if(temp.isFile())
				{
					//Set the time the file was last modified (this should really just be the date of creation for most of these files).
					oldTime=temp.lastModified();
					newTime=System.currentTimeMillis();
					
					//3600000 is the number of milliseconds in an hour.
					//It is thus that this will delete the file if it is more than 2 hours out of date.
					if(Math.abs(oldTime-newTime)>(7200000))
					{
						temp.delete();
					}
				}
			}
		}
	}

}
