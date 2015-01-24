package com.securics.wildlifecapture;

import java.util.List;

import android.app.ActionBar;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class SettingsActivity extends PreferenceActivity {
	
	/**
	 * initializes layout and buttons
	 */
   @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        ActionBar actionBar = getActionBar();
        actionBar.hide();
        
        Button button = new Button(this);
        button.setText("Close Settings");
        button.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View view) {
				SettingsActivity.this.finish();
			}
        	
        });
 
        
        Button burstButton=new Button(this);
        burstButton.setHeight(128);
        burstButton.setText("Do Camera Burst");
        

        setListFooter(button);
        //setListFooter(burstButton);
    }
   
   /**
    * loads headers
    */
    @Override
    public void onBuildHeaders(List<Header> target) {
        loadHeadersFromResource(R.xml.preference_headers, target);
    }

    public static class ExampleSettings extends PreferenceFragment {
    	/**
    	 * adds capture preferences
    	 */
    	@Override
        public void onCreate(Bundle savedInstanceState) {
             super.onCreate(savedInstanceState);
             this.addPreferencesFromResource(R.xml.preferences_example);                    
    	}
    }   	

    public static class CaptureSettings extends PreferenceFragment {
      	/**
      	 * adds list of capture preference 
      	 */
    	@Override
        public void onCreate(Bundle savedInstanceState) {
             super.onCreate(savedInstanceState);
             this.addPreferencesFromResource(R.xml.preferences_capture);                    
    	}
    } 
    public static class CameraSettings extends PreferenceFragment {
    	/**
      	 * adds list of camera preference 
      	 */
    	@Override
        public void onCreate(Bundle savedInstanceState) {
             super.onCreate(savedInstanceState);
             this.addPreferencesFromResource(R.xml.preferences_camera);                    
    	}
    } 
    
    public static class ImageSettings extends PreferenceFragment {
    	/**
      	 * adds list of image preference 
      	 */
    	@Override
        public void onCreate(Bundle savedInstanceState) {
             super.onCreate(savedInstanceState);
             this.addPreferencesFromResource(R.xml.preferences_image);                    
    	}
    } 
    
    /*
    public static class FAQSettings extends PreferenceFragment {
     	 
    	@Override
        public void onCreate(Bundle savedInstanceState) {
             super.onCreate(savedInstanceState);
             this.addPreferencesFromResource(R.xml.preferences_faq);                    
    	}
    } */
    
    public static class GeneralSettings extends PreferenceFragment {
    	/**
      	 * adds list of general preference 
      	 */
    	@Override
        public void onCreate(Bundle savedInstanceState) {
             super.onCreate(savedInstanceState);
             this.addPreferencesFromResource(R.xml.preferences_general);                    
    	}
    } 
    
    
    public static class DeviceSettings extends PreferenceFragment {
    	/**
      	 * adds list of device preference 
      	 */
    	@Override
        public void onCreate(Bundle savedInstanceState) {
             super.onCreate(savedInstanceState);
             this.addPreferencesFromResource(R.xml.preferences_device);                    
    	}
    } 
    /*
    public static class LosSettingsDelDevice extends PreferenceFragment
    {
    	@Override
    	public void onCreate(Bundle savedInstanceState)
    	{
    		super.onCreate(savedInstanceState);
    		this.addPreferencesFromResource(R.xml.preferences_device);
    	}
    }*/
    
    /**Sets settings for the camera's position in space.*/
    public static class CamLocationSettings extends PreferenceFragment
    {
    	/**
      	 * adds list of camera position preference 
      	 */
    	@Override
        public void onCreate(Bundle savedInstanceState) {
             super.onCreate(savedInstanceState);
             this.addPreferencesFromResource(R.xml.preferences_camerapos);   
             //Intent resolutionIntent=new Intent(MainActivity.class, ChangeResolutionActivity.class);
 			
 			 //startActivityForResult(resolutionIntent, 1);
    	}
    }
    
    /**This is the settings fragment for the mode settings: */
    public static class ModeSettings extends PreferenceFragment
    {
    	/**
      	 * adds list of mode preference 
      	 */
    	@Override
    	public void onCreate(Bundle savedInstanceState)
    	{
    		super.onCreate(savedInstanceState);
    		this.addPreferencesFromResource(R.xml.preferences_mode);
    	}
    }
    
 
}
