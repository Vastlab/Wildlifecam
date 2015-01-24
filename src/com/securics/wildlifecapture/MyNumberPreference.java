package com.securics.wildlifecapture;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.TypedArray;
import android.os.Parcelable;
import android.preference.EditTextPreference;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

public class MyNumberPreference extends EditTextPreference {
    private int savedInt;
    private float savedFloat;
    private String preference_string;
    private String myType;
    private String myDefaultValue;
    
    /**
     * This is the constructor called by the inflater
     * @param context
     * @param attrs
     */
    public MyNumberPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setWidgetLayoutResource(R.layout.mypreference_item);
        myType = "int"; //this is default unless something else is stated (right now its float or int only)
        myDefaultValue = "-1";
    }
    
    /**
     * Sets value of myType
     * @param type
     */
    public void setType(String type) {
    	myType = type;
    }

    /**
     * Sets value of myDefaultValue
     * @param value
     */
    public void defaultValue(String value) {
    	myDefaultValue = value;
    }
    
    
    /**
     * Sets value of savedInt, savedFloat, and preference_string
     */
    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
       
		SharedPreferences sharedPreferences = getSharedPreferences();
		preference_string = sharedPreferences.getString(getKey(), myDefaultValue);
		//Log.i("MyNumberPreference","Retreived("+getKey()+"): "+preference_string+ " Default is: "+myDefaultValue);
		
		if(myType.equalsIgnoreCase("INT")) {
			try {
				savedInt = Integer.valueOf(preference_string);				
			} catch(Exception e) {
				savedInt = -1;
			}
			if(savedInt < 0) savedInt = Integer.valueOf(myDefaultValue);
			preference_string = String.valueOf(savedInt);
		}
		else if(myType.equalsIgnoreCase("FLOAT")) {
			try {
				savedFloat = Float.valueOf(preference_string);	
			} catch(Exception e) {
				savedFloat = -1;
			}
			if(savedFloat < 0) savedFloat = Float.valueOf(myDefaultValue);
			preference_string = String.valueOf(savedFloat);
		}
		else {
			if(MainActivity.debugMode) Log.e("MyNumberPreference","Need to set number type");
			preference_string = String.valueOf(0);
		}

        /** Set our custom views inside the layout */
        final TextView myTextView = (TextView) view.findViewById(R.id.mypreference_widget);
        if (myTextView != null) {
            myTextView.setText(String.valueOf(preference_string));
        }
        EditTextPreference p = (EditTextPreference)this.findPreferenceInHierarchy(getKey());
        p.setText(preference_string);
    }

    /**
     * Saved value of savedFloat, savedInt
     */
    @Override
    protected void onDialogClosed(boolean positiveResult) {
    
    	if(positiveResult) {
    		EditTextPreference p = (EditTextPreference)this.findPreferenceInHierarchy(getKey());
    		preference_string = p.getEditText().getText().toString();
    		if (callChangeListener(preference_string)) {
    			setText(preference_string);
    		}
    		
			Editor editor = getEditor();

    		if(myType.equalsIgnoreCase("FLOAT")) {
    			try{
    			savedFloat = Float.parseFloat(preference_string);
    			}catch(Exception e){
    				savedFloat = -1;
    			}
    			editor.putString(getKey(), String.valueOf(savedFloat));
    		}
    		else if(myType.equalsIgnoreCase("INT")) {
    			try{
    			savedInt = Integer.parseInt(preference_string);
    			}catch(Exception e) {
    				savedInt = -1;
    			}
    			//Log.i("MyNumberPreference","SavedInt: "+savedInt);
    			editor.putString(getKey(), String.valueOf(savedInt));
    		}
    		else {
    			if(MainActivity.debugMode) Log.e("MyNumberPreference","Need to set number type");
    		}
			editor.commit();
			notifyChanged();
      	}       
    }
    
    @Override
    protected Object onGetDefaultValue(TypedArray a, int index) {
        /** 
         * This preference type's value type is Integer, so we read the default
         *  value from the attributes as an Integer.
         */
        return a.getString(index);
        		//a.getInteger(index, 0);
    }

    /**
     * Set value to prefrence_string
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
    	//Log.i("MyNumberPreference","onSavedInstanceState(): "+isPersistent());
        final Parcelable superState = super.onSaveInstanceState();
        if (isPersistent()) {
            // No need to save instance state since it's persistent
            return superState;
        }
        return superState;
     }
}
