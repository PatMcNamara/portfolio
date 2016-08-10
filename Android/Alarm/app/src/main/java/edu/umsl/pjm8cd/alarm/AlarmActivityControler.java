package edu.umsl.pjm8cd.alarm;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.view.ViewPager;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.format.DateFormat;
import android.util.Log;

import java.util.Calendar;
import java.util.Date;

import edu.umsl.pjm8cd.alarm.alarm.ListViewFragment;
import edu.umsl.pjm8cd.alarm.timer.TimerViewFragment;

/**
 * Controls the pager.
 */

public class AlarmActivityControler extends AppCompatActivity {
    public static final String OPEN_IN_TIMER = "OpenInTimer";
    private ViewPager pager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.view_pager);

        FragmentManager manager = getSupportFragmentManager();
        pager = (ViewPager)findViewById(R.id.pager);
        pager.setAdapter(new AlarmAdapter(manager));

        pager.setOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener() {
            @Override
            public void onPageSelected(int position) {
                // When swiping between pages, select the
                // corresponding tab.
                try {
                    getSupportActionBar().setSelectedNavigationItem(position);
                }catch(NullPointerException e) {
                    Log.d("CONTROLLER", "NPE tryint to get support action bar.");
                }


            }
        });


        ActionBar actionBar = getSupportActionBar();

        // Specify that tabs should be displayed in the action bar.
        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);

        // Create a tab listener that is called when the user changes tabs.
        ActionBar.TabListener tabListener = new ActionBar.TabListener() {
            public void onTabSelected(ActionBar.Tab tab, FragmentTransaction ft) {
                pager.setCurrentItem(tab.getPosition());
            }

            public void onTabUnselected(ActionBar.Tab tab, FragmentTransaction ft) {
                // Stub needed by ActionBar.TabListener
            }

            public void onTabReselected(ActionBar.Tab tab, FragmentTransaction ft) {
                // Stub needed by ActionBar.TabListener
            }
        };

        actionBar.addTab(actionBar.newTab().setText("Alarm").setTabListener(tabListener));
        actionBar.addTab(actionBar.newTab().setText("Timer").setTabListener(tabListener));

        // If we were started by a user selecting an notification about the timer, start on the timer page.
        if(getIntent().getBooleanExtra(OPEN_IN_TIMER, false)) {
            actionBar.setSelectedNavigationItem(1);
        }
    }

    private class AlarmAdapter extends FragmentPagerAdapter {
        public AlarmAdapter(FragmentManager fm) {
            super(fm);
        }

        @Override
        public Fragment getItem(int position) {
            if(position == 0) {
                return ListViewFragment.newInstance();
            } else {
                return TimerViewFragment.newInstance();
            }
        }

        @Override
        public int getCount() {
            return 2;
        }
    }

    /* This takes the hours and minutes from 00:00 and gives you a date when that time comes up next in the calendar. */
    public static Date getTimeAsDate(int hours, int minutes) {
        Calendar c = Calendar.getInstance();
        c.set(Calendar.HOUR_OF_DAY, hours);
        c.set(Calendar.MINUTE, minutes);
        c.set(Calendar.SECOND, 0);

        if(Calendar.getInstance().after(c)) { // If timer is set for tomorrow.
            c.add(Calendar.DATE, 1);
        }
        return c.getTime();
    }

    /* Creates a string for a given local with the hours and minutes as specified in getTimeAsDate. */
    public static String formatTimeString(int hour, int min, boolean twentyFourHour) {
        String s;
        if(twentyFourHour) {
            s = DateFormat.format("HH:mm", getTimeAsDate(hour, min)).toString();
        } else {
            s = DateFormat.format("hh:mm a", getTimeAsDate(hour, min)).toString();
        }
        return s;
    }
}
