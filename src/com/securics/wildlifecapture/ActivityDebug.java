package com.securics.wildlifecapture;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.sql.Date;
import java.text.DecimalFormat;
import java.text.SimpleDateFormat;
import java.util.Locale;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.BatteryManager;
import android.os.Debug;
import android.os.Handler;
import android.util.Log;

public class ActivityDebug {
		
	private final static String TAG = "MyDebug";

	//Delay for reading CPU Usage (every 30 secs) 
	private final long DELAY = 1000 * 30;

	private String myID = "notset";

	private Handler PSHandler = new Handler();

	
	private final String LOG_FILE;

	/**
	 * Set LOG_FILE name and add ReadingUsage to message Queue
	 * @param lf
	 */
	public ActivityDebug(String lf) {
		LOG_FILE = lf;
		PSHandler.post(ReadingUsage);
	}
	
	/**
	 * Removes pending post from ReadingUsage
	 */
	public void EndPhotoSnapperDebug() {
		PSHandler.removeCallbacks(ReadingUsage);
		
	}
	
	/**
	 * reads CPU usage
	 */
	private float[] readUsage() {

		float answer[] = {-1,-1,-1,-1,-1,-1,-1,-1};

		try {
	        RandomAccessFile reader = new RandomAccessFile("/proc/stat", "r");

	        String load = "";
	        for(int i=0; i < 8; i++) { // up to 8 cores
		        for(int j=0;j<i+2;j++) {
		        	load = reader.readLine();
		        }
		        //Log.i(TAG,"STAT Line1[" + i + "]: " + load);
		        String[] toks = load.split(" ");
			        
		        if(toks[0].substring(0, 3).compareTo("cpu")!=0) break; //only want cpu info, break when past
		        
		        float idle1 = Float.parseFloat(toks[4]);
		        float cpu1 = Float.parseFloat(toks[2]) + Float.parseFloat(toks[3]) + Float.parseFloat(toks[5])
		              + Float.parseFloat(toks[6]) + Float.parseFloat(toks[7]) + Float.parseFloat(toks[8]);
	
		        try {
		            Thread.sleep(360);
		        } catch (Exception e) {}
	
		        reader.seek(0);
		        for(int j=0;j<i+2;j++) {
		        	load = reader.readLine();
		        }
		        //Log.i(TAG,"STAT Line2["+i+"]: " + load);
		        toks = load.split(" ");
	
		        float idle2 = Float.parseFloat(toks[4]);
		        float cpu2 = Float.parseFloat(toks[2]) + Float.parseFloat(toks[3]) + Float.parseFloat(toks[5])
		            + Float.parseFloat(toks[6]) + Float.parseFloat(toks[7]) + Float.parseFloat(toks[8]);
		        float bot = ((cpu2 + idle2) - (cpu1 + idle1))	;
		        if(bot==0.0) answer[i] = 0;
		        else answer[i] = (float)((cpu2 - cpu1) / ((cpu2 + idle2) - (cpu1 + idle1)));
		        //Log.i(TAG,"STAT answer["+i+"] = (" + cpu2 + " - " + cpu1 + ") / ( (" + 
		        //		cpu2 + " + " + idle2 + ") - (" + cpu1 + " + " + idle1 + ") ) = "+ answer[i]);
		        reader.seek(0);

	        }
	        reader.close();

	        String cpuInfo = "\n";
		    for(int i=0; answer[i] > -1 && i < 8;i++) {
		    	cpuInfo += "cpu[" + i + "] = " + answer[i] + "\n";
		        if(MainActivity.debugMode) Log.i(TAG,"STAT cpu[" + i + "]: " + answer[i]);
		    }

		    logMessage("CPU",cpuInfo);
		    
		    		    
		    String myStringArray[]= {"top","-m", "10","-n","1"};
		    Process process = Runtime.getRuntime().exec(myStringArray);
		    InputStream stdout = process.getInputStream();
		    byte[] buffer = new byte[512];
		    int read;
		    String out = new String();
		    //read method will wait forever if there is nothing in the stream
		    //so we need to read it in another way than while((read=stdout.read(buffer))>0)

		    while(true){
		    	read = stdout.read(buffer);
		    	out += new String(buffer, 0, read);
		    	if(read<512){
		    		//we have read everything
		    		break;
		    	}
		    }

		    logMessage("Top",out);
		    
	        
		} catch (IOException ex) {
	    	ex.printStackTrace();
	    }
	    
	    
	    return answer;
	} 	

	/**
	 * display head usage
	 * @param clazz
	 */
	public void logHeap(Class<?> clazz) {
		Double mb = 1048576.0;
	    Double allocated = Double.valueOf(Debug.getNativeHeapAllocatedSize())/mb;
	    Double available = Double.valueOf(Debug.getNativeHeapSize())/mb;
	    Double free = Double.valueOf(Debug.getNativeHeapFreeSize())/mb;
	    DecimalFormat df = new DecimalFormat();
	    df.setMaximumFractionDigits(2);
	    df.setMinimumFractionDigits(2);

	    logMessage(TAG, "debug.heap native: allocated " + df.format(allocated) + "MB of " + df.format(available) + "MB (" + df.format(free) + "MB free) in [" + clazz.getName().replaceAll("edu.vast.wildlifecam.","") + "]");
	    logMessage(TAG, "debug.memory: allocated: " + df.format(Double.valueOf(Runtime.getRuntime().totalMemory())/mb) + "MB of " + df.format(Double.valueOf(Runtime.getRuntime().maxMemory())/mb)+ "MB (" + df.format(Double.valueOf(Runtime.getRuntime().freeMemory())/mb) +"MB free)");
	    System.gc();
	    System.gc();

	    
	}
	
	/**
	 * gets ID
	 * @param ID
	 */
	public void setID(String ID) {
		myID = ID;
	}
	
	/**
	 * writes message in text file LOG_FILE
	 * @param tag
	 * @param message
	 */
	public void logMessage(String tag, String message) {
		
		try{

		    File file = new File(LOG_FILE);
		    SimpleDateFormat sdf = new SimpleDateFormat("MM/dd/yyyy HH:mm:ss",Locale.US);
			String dateString = sdf.format(new Date(System.currentTimeMillis()));
			
		    String logMessage = "[" + dateString + "] " + tag + " [Device: " + myID + "]  " + message + "\n\n";
		    
		    BufferedWriter writer = new BufferedWriter(new FileWriter(file, true));
		    writer.append(logMessage);
		    writer.flush();
		    writer.close();
		    
		    //so we get logs everywhere
		    if(MainActivity.debugMode) Log.i(tag,logMessage);

		} catch (IOException e) {
		    e.printStackTrace();
		}

		
		
		
	}
	
	/**
	 * gets level, scale, status, and teperature of battery
	 */
	public BroadcastReceiver batteryInfoReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {

			//int health = intent.getIntExtra(BatteryManager.EXTRA_HEALTH, 0);
			//int icon_small = intent.getIntExtra(
			//		BatteryManager.EXTRA_ICON_SMALL, 0);
			int level = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0);
			//int plugged = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, 0);
			//boolean present = intent.getExtras().getBoolean(
			//		BatteryManager.EXTRA_PRESENT);
			int scale = intent.getIntExtra(BatteryManager.EXTRA_SCALE, 100);
			int status = intent.getIntExtra(BatteryManager.EXTRA_STATUS, 0);
			//String technology = intent.getExtras().getString(
			//		BatteryManager.EXTRA_TECHNOLOGY);
			int temperature = intent.getIntExtra(
					BatteryManager.EXTRA_TEMPERATURE, 0);
			//int voltage = intent.getIntExtra(BatteryManager.EXTRA_VOLTAGE, 0);

			int percent = (level * 100) / scale;
			float temp = (float) ( (temperature/10.0) * 9.0/5.0 + 32.0);
			
			if(MainActivity.debugMode)
			{
				Log.i(TAG, "Battery Level:" + level);
				Log.i(TAG, "Battery Status: " + status);
				Log.i(TAG, "Battery Percentage: " + percent);
				Log.i(TAG, "Battery temp: " + temp + "F");
			}
			   
			String battery = "level " + level + ", status: " + status + ", percentage: " + 
								percent + ", temp: " + temp + "F\n\n";
			 
			logMessage("Battery",battery);
				
			/*
			 * batteryInfo.setText( "Health: "+health+"\n"+
			 * "Icon Small:"+icon_small+"\n"+ "Level: "+level+"\n"+
			 * "Plugged: "+plugged+"\n"+ "Present: "+present+"\n"+
			 * "Scale: "+scale+"\n"+ "Status: "+status+"\n"+
			 * "Technology: "+technology+"\n"+ "Temperature: "+temperature+"\n"+
			 * "Voltage: "+voltage+"\n");
			 * imageBatteryState.setImageResource(icon_small);
			 */
		}
	};

	/**
	 * runs ReadUsage ina different thread and post it to the message queue
	 */
	private Thread ReadingUsage = new Thread() {

		@Override
		public void run() {
			readUsage();
			PSHandler.postDelayed(ReadingUsage, DELAY);
		}		
	};
}