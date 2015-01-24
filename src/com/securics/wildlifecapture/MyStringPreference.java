package com.securics.wildlifecapture;

import android.content.Context;
import android.content.res.TypedArray;
import android.os.Parcel;
import android.os.Parcelable;
import android.preference.EditTextPreference;
import android.util.AttributeSet;
import android.view.View;
import android.widget.TextView;

public class MyStringPreference extends EditTextPreference {
    private String preference_string;

    /** 
     * This is the constructor called by the inflater
     * @param context
     * @param attrs
     */
    public MyStringPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setWidgetLayoutResource(R.layout.mypreference_item);
    }

    /**
     * Sets test to preference_string if myTextView is null.
     * If not, declare EditTextPreference p as an extension
     * of shared preference and let p save the value of preference_string
     * to it
     */
    @Override
    protected void onBindView(View view) {
        super.onBindView(view);

        /** Set our custom views inside the layout */
        final TextView myTextView = (TextView) view.findViewById(R.id.mypreference_widget);
        if (myTextView != null) {
            myTextView.setText(String.valueOf(preference_string));
        }
        EditTextPreference p = (EditTextPreference)this.findPreferenceInHierarchy(getKey());
        p.setText(preference_string);
    }

    /**
     * If positiveResult is true, and preference_string has changed, save preference_string
     * to SharedPreference
     */
    @Override
    protected void onDialogClosed(boolean positiveResult) {
    
    	if(positiveResult) {
    		EditTextPreference p = (EditTextPreference)this.findPreferenceInHierarchy(getKey());
    		preference_string = p.getEditText().getText().toString();
    		if (callChangeListener(preference_string)) {
    			setText(preference_string);
    		}
    		persistString(preference_string);
    		notifyChanged();
     	}       
    }
    
    @Override
    protected Object onGetDefaultValue(TypedArray a, int index) {
        /**
         * This preference type's value type is Integer, so we read the default
         * value from the attributes as an Integer.
         */
        return a.getString(index);
        		//a.getInteger(index, 0);
    }

    /**
     * Intializes preference_string
     */
    @Override
    protected void onSetInitialValue(boolean restoreValue, Object defaultValue) {

    	if (restoreValue) {
            // Restore state
        	preference_string = this.getPersistedString(preference_string);
        } else {
            // Set state
            String value = (String) defaultValue;
            preference_string = value;
            persistString(value);
        }
    }

    @Override
    protected Parcelable onSaveInstanceState() {
        /**
         * Suppose a client uses this preference type without persisting. We
         * must save the instance state so it is able to, for example, survive
         * orientation changes.
         */

        final Parcelable superState = super.onSaveInstanceState();
        if (isPersistent()) {
            // No need to save instance state since it's persistent
            return superState;
        }

        // Save the instance state
        final SavedState myState = new SavedState(superState);
        myState.myurl = preference_string;
        return myState;
    }

    /**
     * Restores the instance state if it is not already
     */
    @Override
    protected void onRestoreInstanceState(Parcelable state) {
        if (!state.getClass().equals(SavedState.class)) {
            // Didn't save state for us in onSaveInstanceState
            super.onRestoreInstanceState(state);
            return;
        }

        // Restore the instance state
        SavedState myState = (SavedState) state;
        super.onRestoreInstanceState(myState.getSuperState());
        preference_string = myState.myurl;
	    notifyChanged();

    }

    /**
     * SavedState, a subclass of {@link BaseSavedState}, will store the state
     * of MyPreference, a subclass of Preference.
     * <p>
     * It is important to always call through to super methods.
     */
    private static class SavedState extends BaseSavedState {
        String myurl;

        /**
         * initializes source and set myurl
         * @param source
         */
        public SavedState(Parcel source) {
            super(source);

            // Restore the click counter
            myurl = source.readString();
        }

        /**
         * initializes dest and writes myurl to dest
         */
        @Override
        public void writeToParcel(Parcel dest, int flags) {
            super.writeToParcel(dest, flags);

            // Save the click counter
            dest.writeString(myurl);
        }

        /**
         * initializes superState for this class
         * @param superState
         */
        public SavedState(Parcelable superState) {
            super(superState);
        }
    }
}
