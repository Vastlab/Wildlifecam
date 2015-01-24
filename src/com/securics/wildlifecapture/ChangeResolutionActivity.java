package com.securics.wildlifecapture;

import java.util.List;

import android.app.Activity;
import android.app.ListActivity;
import android.hardware.Camera;
import android.hardware.Camera.Size;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;

public class ChangeResolutionActivity extends ListActivity {
	
	private ListView internalView;
	private Camera camCopy;
	private Size picSize;
	private List<Size> sizeList;
	private String[] sizeNames;
	
	/**
	 * This should probably be able to potentially maybe get the sizeList from the non-static copy of the 
	 * static camera contained in MainActivity.
	 */
	public void getSizeData()
	{
		//camCopy=getCameraInstance();
		Camera.Parameters params=camCopy.getParameters();
		sizeList=camCopy.getParameters().getSupportedPictureSizes();
		
	}
	
	/**
	 * Add all picture sizes to sizeNames array
	 */
	public void sizeToString()
	{
		int i;
		
		sizeNames=new String[sizeList.size()];
		
		for(i=0;i<sizeList.size();i++)
		{
			if(sizeList.get(i).equals(camCopy.getParameters().getPictureSize()))
			{
				sizeNames[i]=sizeList.get(i).width+" by "+sizeList.get(i).height+" --Current setting.";
			}
			
			else
			{
				sizeNames[i]=sizeList.get(i).width+" by "+sizeList.get(i).height;
			}
			System.err.println("SizeNames at i is: "+sizeNames[i]);
			//Log.i("TagMeister! ", "SizeNames at "+i+" is: "+sizeNames[i]);
		}
	}
	
	/**
	 * set list adapter
	 */
	public void setListView()
	{
		/**Set up the array adapter.*/
		//internalView.setAdapter(new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, sizeNames));
		setListAdapter(new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, sizeNames));
	}
	
	/**
	 * set layout
	 */
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_change_resolution);
		
	}

	/**
	 * get instance of camera
	 */
	@Override
	protected void onResume() {
		super.onResume();
			//internalView=(ListView) findViewById(R.id.listView);
		//This should technically work as a way of sharing data.
		
		camCopy=getCameraInstance();
				
		if(internalView==null)
		{
			System.err.println("InternalView is null!!");
		}
		
		if(camCopy != null) {
			getSizeData();
			sizeToString();
			setListView();
		}

	}
	
	/**
	 * Create an option menu
	 */
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		/** Inflate the menu; this adds items to the action bar if it is present.*/
		getMenuInflater().inflate(R.menu.change_resolution, menu);
		return true;
	}
	
	/**
	 * Set resolution on MainActivity
	 */
	public void onListItemClick(ListView tmp, View v, int pos, long id)
	{
		//MainActivity.publicParams=MainActivity.outputCam.getParameters();
		//MainActivity.publicParams.setPictureSize(sizeList.get(pos).width, sizeList.get(pos).height);
		MainActivity.publicWidth=sizeList.get(pos).width;
		MainActivity.publicHeight=sizeList.get(pos).height;
		System.err.println("New resolution set! New res: "+sizeNames[pos]);
		//MainActivity.outputCam=null;
		
		camCopy.release();
		camCopy=null;
		ChangeResolutionActivity.this.finish();
	}
	
	/**
	 * Attempt to get a camera.
	 */
	private Camera getCameraInstance() 
	{
		Camera c = null;

		try {
			//CAM_3D is just 0 anyway.
			c = Camera.open(0);
			//c=Camera.open();
		} catch (Exception e) {
			if(MainActivity.debugMode) 
				Log.e("WildLifeCapture_CameraResolution","ERROR: Camera instance could not be obtained from OS!");
		}
		
		return c;
	}
}
