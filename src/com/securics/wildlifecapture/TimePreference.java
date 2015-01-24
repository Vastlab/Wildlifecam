package com.securics.wildlifecapture;

import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

//import edu.vast.wildlifecam.R;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.TypedArray;
import android.os.Parcel;
import android.os.Parcelable;
import android.preference.DialogPreference;
import android.preference.Preference.BaseSavedState;
import android.text.format.DateFormat;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.TimePicker;

public class TimePreference extends DialogPreference {
	private Calendar calendar;
	private TimePicker picker = null;
	private int theHour, theMinute;
	private String hour = "theHour" + getKey();
	private String minute = "theMinute" + getKey();

	/**
	 * sets context
	 * @param ctxt
	 */
	public TimePreference(Context ctxt) {
		this(ctxt, null);
	}

	/**
	 * set context and attribute
	 * @param ctxt
	 * @param attrs
	 */
	public TimePreference(Context ctxt, AttributeSet attrs) {
		this(ctxt, attrs, 0);
	}

	/**
	 * set context, attrs, and defstype. Set positive and negative
	 * button and set calendar as a new instance of GregorianCalendar
	 * @param ctxt
	 * @param attrs
	 * @param defStyle
	 */
	public TimePreference(Context ctxt, AttributeSet attrs, int defStyle) {
		super(ctxt, attrs, defStyle);

		setPositiveButtonText("Set");
		setNegativeButtonText("Cancel");
		calendar = new GregorianCalendar();
	}

	/**
	 * sets picker to the preference for TimePicker and return it
	 */
	@Override
	protected View onCreateDialogView() {
		// Log.i("TimePref","onCreateDialogView: " + theHour +":" + theMinute);
		picker = new TimePicker(getContext());
		return (picker);
	}

	/**
	 * set value for picker
	 */
	@Override
	protected void onBindDialogView(View v) {
		super.onBindDialogView(v);
		SharedPreferences sharedPreferences = getSharedPreferences();
		String startKey = this.getContext().getResources()
				.getString(R.string.start_key);
		// you're the stop key, check to make sure you're greater than start
		// key values
		if(getKey().equals(startKey)) {
			theHour = sharedPreferences.getInt(hour, 6);
		} else { // stop key
			theHour = sharedPreferences.getInt(hour, 18);
		}
			theMinute = sharedPreferences.getInt(minute, 0);

		// Log.i("TimePref","onBindDialogView: " + theHour +":" + theMinute);
		picker.setCurrentHour(theHour);
		picker.setCurrentMinute(theMinute);
	}

	/**
	 * get value for theHour and theMinute and save it to the editor
	 */
	@Override
	protected void onDialogClosed(boolean positiveResult) {
		super.onDialogClosed(positiveResult);

		if (positiveResult) {

			calendar.set(Calendar.HOUR_OF_DAY, picker.getCurrentHour());
			calendar.set(Calendar.MINUTE, picker.getCurrentMinute());

			// Log.i("TimePref","PositiveResult: " + picker.getCurrentHour() +
			// ":"+ picker.getCurrentMinute());

			setSummary(getSummary());
			if (callChangeListener(calendar.getTimeInMillis())) {
				theHour = picker.getCurrentHour();
				theMinute = picker.getCurrentMinute();
				 //Log.i("TimePref","CallChangeListener: " + theHour +":" + theMinute);

			}

			String stopKey = this.getContext().getResources()
					.getString(R.string.stop_key);
			String startKey = this.getContext().getResources()
					.getString(R.string.start_key);
			// you're the stop key, check to make sure you're greater than start
			// key values
			int startH, startM, stopH, stopM;
			SharedPreferences sharedPreferences = getSharedPreferences();
			if(getKey().equals(startKey)) {
				startH = picker.getCurrentHour();
				startM = picker.getCurrentMinute();
			}
			else {
				startH = sharedPreferences.getInt("theHour" + startKey, 6);
				startM = sharedPreferences.getInt("theMinute" + startKey, 0);
			}
			if(getKey().equals(stopKey)) {
				stopH = picker.getCurrentHour();
				stopM = picker.getCurrentMinute();
			}
			else {
				stopH = sharedPreferences.getInt("theHour" + stopKey, 18);
				stopM = sharedPreferences.getInt("theMinute" + stopKey, 0);
			}

			if ((stopH * 60 * 60 + stopM * 60) < (startH * 60 * 60 + startM * 60)) {
				//Log.e("TimePrefs", startH + ":" + startM + " is after " + stopH + ":"	+ stopM);
				if(getKey().equals(stopKey)) {
					Editor editor = getEditor();
					editor.putInt("theHour" + startKey, theHour);
					editor.putInt("theMinute" + startKey, theMinute);
					editor.commit();
				}
				else {
					Editor editor = getEditor();
					editor.putInt("theHour" + stopKey, theHour);
					editor.putInt("theMinute" + stopKey, theMinute);
					editor.commit();
				}
				
				//return;
			}

			//Log.i("TimePref","Persisting: " + theHour +":" + theMinute);
			Editor editor = getEditor();
			editor.putInt(hour, theHour);
			editor.putInt(minute, theMinute);
			editor.commit();

		}
		setSummary(getSummary());

	}

	/**
	 * returns the value of TypedArray a in the index if there is no value return 4
	 */
	@Override
	protected Object onGetDefaultValue(TypedArray a, int index) {
		// Log.i("TimePref","Got Default Value" + a.getInteger(index, 4));
		return a.getInteger(index, 4);
	}

	/**
	 * Sets calendar to proper values and save those values to editor
	 */
	@Override
	protected void onSetInitialValue(boolean restoreValue, Object defaultValue) {

		SharedPreferences sharedPreferences = getSharedPreferences();
		int startHour=6;

		String stopKey = this.getContext().getResources()
				.getString(R.string.stop_key);
		if(getKey().equals(stopKey)) startHour = 18;
		
		theHour = sharedPreferences.getInt(hour, startHour);
		theMinute = sharedPreferences.getInt(minute, 0);

		// Log.i("TimePrefs","Initial Value: "+theHour+":"+theMinute);
		// Log.i("TimePrefs","Restore Value: "+restoreValue);

		if (restoreValue) {
			if(MainActivity.debugMode) Log.i("TimePrefs", "RestoreValue ");
			calendar.set(Calendar.HOUR_OF_DAY, theHour);
			calendar.set(Calendar.MINUTE, theMinute);

		} else {
			if (defaultValue == null) {
				// Log.i("TimePrefs","defaultValue is null");
				calendar.setTimeInMillis(System.currentTimeMillis());
			} else {
				// Log.i("TimePrefs","defaultValue: "+defaultValue);
				calendar.setTimeInMillis(Long.parseLong((String) defaultValue));
			}
			Editor editor = getEditor();
			editor.putInt(hour, theHour);
			editor.putInt(minute, theMinute);
			editor.commit();
		}
		setSummary(getSummary());
	}

	/**
	 * gets value for theHour and theMinute, set calendar to the values
	 * and return date format of calendar
	 */
	@Override
	public CharSequence getSummary() {
		// Log.i("TimePrefs","getSummary");
		if (calendar == null) {
			return null;
		}

		String startKey = this.getContext().getResources()
				.getString(R.string.start_key);
		// you're the stop key, check to make sure you're greater than start
		// key values
		SharedPreferences sharedPreferences = getSharedPreferences();
		if(getKey().equals(startKey)) {		
			theHour = sharedPreferences.getInt(hour, 6);
		} else { //stop key
			theHour = sharedPreferences.getInt(hour, 18);
		}
		theMinute = sharedPreferences.getInt(minute, 0);

		calendar.set(Calendar.HOUR_OF_DAY, theHour);
		calendar.set(Calendar.MINUTE, theMinute);

		return DateFormat.getTimeFormat(getContext()).format(
				new Date(calendar.getTimeInMillis()));
	}

	@Override
	protected Parcelable onSaveInstanceState() {
		final Parcelable superState = super.onSaveInstanceState();
		// Check whether this Preference is persistent (continually saved)
		// Log.i("TimePref","onSaveInstanceState: "+theHour+":"+theMinute);

		if (isPersistent()) {
			// No need to save instance state since it's persistent, use
			// superclass state
			// Log.i("TimePref","it's persistent: ");
			Editor editor = getEditor();
			editor.putInt(hour, theHour);
			editor.putInt(minute, theMinute);
			editor.commit();

			return superState;
		}

		// Log.i("TimePref","saving state.");
		/** Create instance of custom BaseSavedState*/
		final SavedState myState = new SavedState(superState);
		/** Set the state's value with the class member that holds current*/
		// setting value
		myState.myHour = theHour;
		myState.myMinute = theMinute;

		return myState;
	}

	/**
	 * Set superState from the parameter to the subclass
	 */
	@Override
	protected void onRestoreInstanceState(Parcelable state) {

		// Log.i("TimePref","onRestoreInstanceState: "+theHour+":"+theMinute);
		/** Check whether we saved the state in onSaveInstanceState */
		if (state == null || !state.getClass().equals(SavedState.class)) {

			/** Didn't save the state, so call superclass*/
			super.onRestoreInstanceState(state);
			return;
		}

		/** Cast state to custom BaseSavedState and pass to superclass*/
		SavedState myState = (SavedState) state;
		super.onRestoreInstanceState(myState.getSuperState());

		/** Set this Preference's widget to reflect the restored state*/
		theHour = myState.myHour;
		theMinute = myState.myMinute;
		// theMinute = myState.myMinute;
		// Log.i("TimePref","onRestoreInstanceState (!null): "+theHour+":"+theMinute);
		picker.setCurrentHour(theHour);
		picker.setCurrentMinute(theMinute);

	}

	/**
	 * SavedState, a subclass of {@link BaseSavedState}, will store the state of
	 * MyPreference, a subclass of Preference.
	 * <p>
	 * It is important to always call through to super methods.
	 */
	private static class SavedState extends BaseSavedState {
		/**Member that holds the setting's value
		 * Change this data type to match the type saved by your Preference
		 */
		int myHour;
		int myMinute;

		public SavedState(Parcelable superState) {
			super(superState);
			// Log.i("TimePref","SavedState superState: "+myHour);
		}

		/**
		 * Set source from the parameter to the subclass
		 * @param source
		 */
		public SavedState(Parcel source) {
			super(source);
			/** Get the current preference's value */
			myHour = source.readInt(); // Change this to read the appropriate
										// data type
			myMinute = source.readInt(); // Change this to read the appropriate
											// data type
			// Log.i("TimePref","SavedState parcel: "+myHour);
		}

		/**
		 * set dest and flags from the parameter to the subclass
		 */
		@Override
		public void writeToParcel(Parcel dest, int flags) {
			super.writeToParcel(dest, flags);
			// Log.i("TimePref","writeToParcel: "+myHour);
			/** Write the preference's value */
			dest.writeInt(myHour); /** Change this to write the appropriate data type*/
			dest.writeInt(myMinute); /** Change this to write the appropriate data type */
		}

		/** Standard creator object using an instance of this class */
		@SuppressWarnings("unused")
		public static final Parcelable.Creator<SavedState> CREATOR = new Parcelable.Creator<SavedState>() {

			public SavedState createFromParcel(Parcel in) {
				return new SavedState(in);
			}

			public SavedState[] newArray(int size) {
				return new SavedState[size];
			}
		};
	}

}