package com.securics.wildlifecapture;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.ListPreference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.view.View;

public class CameraPreference extends PreferenceCategory {

	Context context;
	AttributeSet attr;
	ArrayList<MyList> theList;

	/**
	 * Sets class context, attrs using context and attrs.
	 * Creates and insert values in theList
	 * @param context
	 * @param attrs
	 */
	public CameraPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
		this.context = context;
		this.attr = attrs;

		theList = new ArrayList<MyList>();
		//MyList(3) will be a way of getting to camera vert. height.
		//Upon further review, this setting cannot be done here due to the way
		//this is designed.
		//theList.add(new MyList(3));
		theList.add(new MyList(0));
		theList.add(new MyList(1));
		theList.add(new MyList(2));

	}

	/**
	 * Calls addMyList to ad theList to MyList
	 */
	@Override
	protected void onBindView(View view) {
		super.onBindView(view);

		addMyList(theList);
	}
	
	/**
	 * Add teList to MyList
	 * @param lst
	 */
	private void addMyList(ArrayList<MyList> lst) {

		for (MyList l : lst) {
			this.addItemFromInflater(l);
		}

	}

	public class MyList extends ListPreference {

		String myTitle;
		CharSequence[] entries;
		CharSequence[] entryValues;

		/**
		 * Set entries, entry values, title to myTitle and button text
		 * @param m
		 */
		public MyList(int m) {
			this(context, attr);
			myMode(m);
			this.setDialogTitle(myTitle);
			setEntries(entries);
			setEntryValues(entries);
			this.setTitle(myTitle);
			setNegativeButtonText("OK");
			
			}

		/**
		 * Set entries, entry values, title to myTitle and button text
		 * @param context
		 * @param attrs
		 */
		public MyList(Context context, AttributeSet attrs) {
			super(context, attrs);
			setDialogLayoutResource(R.layout.mypreference_item);

		}

		/**
		 * Uses a switch to set variables accordingly
		 * @param m
		 */
		private void myMode(int m) {
			Set<String> set = null;
			SharedPreferences prefs = PreferenceManager
					.getDefaultSharedPreferences(context
							.getApplicationContext());
			
			switch (m) {
			case 0:
				myTitle = "Focus Mode";

				set = prefs
						.getStringSet("CamPrefsFocus", new HashSet<String>());
				entries = set.toArray(new CharSequence[set.size()]);
				this.setKey("FocusMode");
				break;
			case 1:
				myTitle = "Scene Mode";
				set = prefs
						.getStringSet("CamPrefsScene", new HashSet<String>());
				entries = set.toArray(new CharSequence[set.size()]);
				this.setKey("SceneMode");
				break;
			case 2:
				myTitle = "White Balance";
				set = prefs.getStringSet("CamPrefsWBalance",
						new HashSet<String>());
				entries = set.toArray(new CharSequence[set.size()]);
				this.setKey("WhiteBalance");
				break;
			default:
				break;
			}
		}
	}
}