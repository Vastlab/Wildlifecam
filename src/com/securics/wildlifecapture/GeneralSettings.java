package com.securics.wildlifecapture;

import android.content.Context;
import android.preference.PreferenceCategory;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

public class GeneralSettings extends PreferenceCategory {

	Context context;
	AttributeSet attr;
	String[] params = null;
	boolean loaded;
	
	/**
	 * Depending on tittle from context, input the array from
	 * either generalstuff or general_swindow
	 * @param context
	 * @param attrs
	 */
	public GeneralSettings(Context context, AttributeSet attrs) {
		super(context, attrs);
		this.context = context;
		this.attr = attrs;

		if(this.getTitle().equals(context.getResources().getString(R.string.general_stuff))) {
			params = context.getResources().getStringArray(R.array.General_Squirrel_Params);
		}
		/*else if(this.getTitle().equals(context.getResources().getString(R.string.general_swindow))) {
			params = context.getResources().getStringArray(R.array.Sliding_Window_Params);
		}*/
		else {
			if(MainActivity.debugMode) Log.e("GeneralSettings","Shouldn't have been here. Params not set");
		}
		loaded = false;

	}

	/**
	 * calls loadParams
	 */
	@Override
	protected void onBindView(View view) {
		super.onBindView(view);
		//Log.i("GeneralSettings", "onBind");

		loadParams();
	}

	/**
	 * uses MyNumberPreference to load values to mp
	 */
	private void loadParams() {
		
		if(loaded) return;
		loaded = true;
		for(int i=0; i < params.length; i+=2) {
			String p = params[i];
			//String d = params[i+1];
			MyNumberPreference mp = new MyNumberPreference(context,attr);
			if(p.equalsIgnoreCase("maxratio") || p.equalsIgnoreCase("scalefactor") || p.equalsIgnoreCase("fudge") )
				mp.setType("float");
			mp.setKey(p);
			mp.setTitle(p);
			//Log.i("GeneralSettings","Setting Default of: "+d);
			mp.defaultValue(params[i+1]);
			this.addItemFromInflater(mp);
		}
	}


}