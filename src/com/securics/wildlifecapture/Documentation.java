package com.securics.wildlifecapture;

import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.Activity;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.webkit.WebView;

/**
 * This activity is an online documentation engine for the program.
 * The way this activity displays documentation is via a package-integrated web page.
 * It may be possible to eventually have a tabbed help interface that loads individual 
 * HTML pages for every section, however that will not be implemented soon.
 */
public class Documentation extends Activity {

	WebView internalWebView;
	//TabHost internalTabHost;
	//FragmentPagerAdapter adapterMeister;
	//final ActionBar internalActionBar;
	//WebView internalWebView;
	
	/**
	 * Set activity layout and action bar
	 */
	@Override
	protected void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_documentation);
		
		final ActionBar internalActionBar=getActionBar();
		
		int i;
		
		internalActionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
		//internalWebView=(WebView) findViewById(R.id.webview);
		
		internalWebView=(WebView) findViewById(R.id.webview);
		//internalWebView.loadUrl("file:///android_asset/html/Manual.html");
		//internalWebView=new WebView(this, null);
		//internalTabHost=(TabHost) findViewById(android.R.id.tabhost);
		//internalTabHost.toString();
		
		ActionBar.TabListener tabListenerMeister=new ActionBar.TabListener() 
		{
			
			@Override
			public void onTabUnselected(Tab tab, FragmentTransaction ft) {
				// TODO Auto-generated method stub
				
			}
			
			@Override
			public void onTabSelected(Tab tab, FragmentTransaction ft) 
			{
				/**Note: There should be a table of if statements regarding the tab names
				 * and which page to load.
				 */
				internalWebView.loadUrl("file:///android_asset/html/Manual.html");
				//tab.setCustomView(internalWebView);
				
				if(tab.getText().equals("Section 1"))
				{
					internalWebView.loadUrl("file:///android_asset/html/Section_0.html");
				}
				
				else if(tab.getText().equals("Section 2"))
				{
					internalWebView.loadUrl("file:///android_asset/html/Section_1.html");
				}
				
				else if(tab.getText().equals("Section 3"))
				{
					internalWebView.loadUrl("file:///android_asset/html/Section_2.html");
				}
				
				else if(tab.getText().equals("Section 4"))
				{
					internalWebView.loadUrl("file:///android_asset/html/Section_3.html");
				}
				
				else if(tab.getText().equals("FAQ"))
				{
					internalWebView.loadUrl("file:///android_asset/html/FAQ.html");
				}
				
				else if(tab.getText().equals("Additional Information"))
				{
					internalWebView.loadUrl("file:///android_asset/html/AddtlInfo.html");
				}
				
				else if(tab.getText().equals("Appendix 1"))
				{
					internalWebView.loadUrl("file:///android_asset/html/Appdx_1.html");
				}
			}
			
			@Override
			public void onTabReselected(Tab tab, FragmentTransaction ft) {
				// TODO Auto-generated method stub
				
			}
		};
		
		//The documentation goes from section 0 to 3.
		for(i=0;i<4;i++)
		{
			internalActionBar.addTab(internalActionBar.newTab().setText("Section "+(i+1)).setTabListener(tabListenerMeister));
		}
		
		internalActionBar.addTab(internalActionBar.newTab().setText("FAQ").setTabListener(tabListenerMeister));
		internalActionBar.addTab(internalActionBar.newTab().setText("Additional Information").setTabListener(tabListenerMeister));
		internalActionBar.addTab(internalActionBar.newTab().setText("Appendix 1").setTabListener(tabListenerMeister));
	}

	/**
	 * Creates a menu
	 */
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.documentation, menu);
		return true;
	}
	
	/**
	 * Listener for onCreateOptionsMenu
	 */
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		int itemId = item.getItemId();
		if (itemId == R.id.action_settings) {
			Intent settingsIntent = new Intent(this, SettingsActivity.class);
			Documentation.this.startActivityForResult(settingsIntent, 1);
		} else {
		}
		item=null;
		return super.onOptionsItemSelected(item);
		}
	}