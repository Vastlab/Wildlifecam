package com.securics.wildlifecapture;

import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences.Editor;
import android.preference.DialogPreference;
import android.preference.PreferenceCategory;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

public class ResetDefaults extends PreferenceCategory {

	Context context;
	AttributeSet attr;
	MyFAQ m;
	
	/**
	 * Initialize context, attr and m
	 * @param context
	 * @param attrs
	 */
	public ResetDefaults(Context context, AttributeSet attrs) {
		super(context, attrs);
		this.context = context;
		this.attr = attrs;
	
	
		m = new MyFAQ();
		
	}

	/**
	 * Binds view from the parameter and m
	 */
	@Override
	protected void onBindView(View view) {
		super.onBindView(view);
		this.addItemFromInflater(m);
	}


	public class MyFAQ extends DialogPreference {

		String question, answer;
		
		/**
		 * Initialize question and answer. Set dialog title and title to variable question, set summary to Reset
		 * All Settings, and text to variable answer
		 */
		public MyFAQ() {
			this(context,attr);
			  this.setNegativeButtonText("Cancel");
			setDialogLayoutResource(R.layout.faq_item);
			question = "Reset ALL Settings.";
			answer = "Are you sure you wish to reset all settings? This cannot be undone.";			 
			setDialogTitle(question);
			setTitle(question);
			setSummary("Reset All Settings.");
			setText(answer);
		}
		
		/**
		 * Uses function super to call parent constructor with parameters context, attrs, and defStyle
		 * @param context
		 * @param attrs
		 * @param defStyle
		 */
		public MyFAQ(Context context, AttributeSet attrs, int defStyle) {
			super(context, attrs, defStyle);
		}

		/**
		 * Uses function super to call parent constructor with parameters context and attrs
		 * @param context
		 * @param attrs
		 */
		public MyFAQ(Context context, AttributeSet attrs) {
			super(context, attrs);
		}
		
		private TextView mEditText;	
		 
		@Override
		protected View onCreateDialogView() {
		  View root = super.onCreateDialogView();
		  mEditText = (TextView) root.findViewById(R.id.edit);
		  return root;
		}
		
		private String mText;
		 /**
		  * Set mText to text
		  * @param text
		  */
		public void setText(String text) {
		  mText = text;
		}
			
		/**
		 * Returns mText
		 * @return
		 */
		public String getText() {
		  return mText;
		}
		 		
		/**
		 * Set mText as text for mEditText
		 */
		@Override
		protected void onBindDialogView(View view) {
		  mEditText.setText(mText);
		}
		
		/**
		 * Manage feedback from buttons
		 */
		@Override
		public void onClick(DialogInterface dialog, int which) {
		  switch(which) {
		    case DialogInterface.BUTTON_POSITIVE: // User clicked OK!
		      mText = mEditText.getText().toString();
		      callChangeListener(mText);
		      resetAllSettings();
		      Log.i("Reset","User Clicked OK!");
		      break;
		    case DialogInterface.BUTTON_NEGATIVE:
		    	Log.i("Reset","User Clicked Cancel");
		    	break;
	    	default:
	    		break;
		  }
		  super.onClick(dialog, which);
		}
		
		/**
		 * Resets squirrel and window params, camera, schedule, capture Schedule settings and file 	and folder locations to default.
		 * @return
		 */
		private boolean resetAllSettings() {
			String params[];
			//SharedPreferences sharedPreferences = getSharedPreferences();
			Editor editor = getEditor();

			/**General Squirrel Params*/
			params = context.getResources().getStringArray(R.array.General_Squirrel_Params);
			for(int i=0; i < params.length; i+=2) {
				String p = params[i];
				String d = params[i+1];
				editor.putString(p, d);
			}

			/**Sliding Window Params*/
			/*params = context.getResources().getStringArray(R.array.Sliding_Window_Params);
			for(int i=0; i < params.length; i+=2) {
				String p = params[i];
				String d = params[i+1];
				editor.putString(p, d);
			}*/

			/**Camera Settings */
			editor.putString("FocusMode","auto");
			editor.putString("SceneMode","auto");
			editor.putString("WBalanceMode","auto");
			
			/**Capture Schedule Settings*/
			String myHour = "theHour"+context.getResources().getString(R.string.start_key);
			String myMinute = "theMinute"+context.getResources().getString(R.string.start_key);
			editor.putInt(myHour, 6);
			editor.putInt(myMinute, 0);
			myHour = "theHour"+context.getResources().getString(R.string.stop_key);
			myMinute = "theMinute"+context.getResources().getString(R.string.stop_key);
			editor.putInt(myHour, 18);
			editor.putInt(myMinute, 0);
			
			/**File and Folder locations */
			String myKey, myValue;
			myKey = context.getResources().getString(R.string.gallery_key);
			myValue = context.getResources().getString(R.string.gallery_default);
			editor.putString(myKey, myValue);
			myKey = context.getResources().getString(R.string.chips_key);
			myValue = context.getResources().getString(R.string.chips_default);
			editor.putString(myKey, myValue);
			myKey = context.getResources().getString(R.string.model_key);
			myValue = context.getResources().getString(R.string.model_default);
			editor.putString(myKey, myValue);
			myKey = context.getResources().getString(R.string.processlog_key);
			myValue = context.getResources().getString(R.string.processlog_default);
			editor.putString(myKey, myValue);
			myKey = context.getResources().getString(R.string.applog_key);
			myValue = context.getResources().getString(R.string.applog_default);
			editor.putString(myKey, myValue);
			myKey = context.getResources().getString(R.string.output_key);
			myValue = context.getResources().getString(R.string.output_default);
			editor.putString(myKey, myValue);
			
			
			
			//theHour = sharedPreferences.getInt(hour, startHour);
			

			editor.commit();
			
			return true;
		}

	}

}