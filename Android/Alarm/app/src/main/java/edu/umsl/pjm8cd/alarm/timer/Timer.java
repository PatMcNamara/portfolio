package edu.umsl.pjm8cd.alarm.timer;

import android.os.Handler;
import android.util.Log;

import java.util.Calendar;
import java.util.Date;

/**
 * Created by Pat on 5/12/2016.
 *
 * Model to handle the timer. Also is a runnable for managing timer updated.
 */
public class Timer implements Runnable {
    interface TimerDelegate {
        void updateTime();
        void timerFinished();
    }
    TimerDelegate del;
    public void setDelegate(TimerDelegate delagate) {
        del = delagate;
    }

    private Date timeFinished;
    private Date timePaused; // Should be null if timer is running.

    private Handler handler;

    public Timer(TimerDelegate delegate) {
        timeFinished = null;
        handler = new Handler();
        setDelegate(delegate);
    }
    public Timer(Date finish) {
        timeFinished = finish;
        handler = new Handler();
    }
    public Timer(TimerDelegate delegate, Date finish) {
        this(finish);
        setDelegate(delegate);
    }

    public void setTimeFinished(Date timeFinished) {
        handler.removeCallbacks(this); // We are starting a new time, reset the timer.
        this.timeFinished = timeFinished;
    }

    public long getTimeFinished() {
        return timeFinished.getTime();
    }

    public String getTimeString() {
        return formatTimeString(timeFinished.getTime() - (new Date()).getTime());
    }

    /* Takes in a number of milliseconds and gives a time string of min:sec:milliseconds. */
    public static String formatTimeString(long milliseconds) {
        return String.format("%02d:%02d:%02d", (milliseconds / 3600000) % 24, (milliseconds / 60000) % 60, (milliseconds / 1000) % 60);
    }

    public void threadTryStart() {
        if(timeFinished != null && timePaused == null) {
            threadStart();
        }
    }

    public void threadStart() {
        if(timePaused != null) { // We are resuming from a pause.
            long timeEllapsed = System.currentTimeMillis() - timePaused.getTime();
            timeFinished.setTime(timeEllapsed + timeFinished.getTime());
            timePaused = null;
        }
        handler.postDelayed(this, 500);
    }

    public void threadPause() {
        timePaused = new Date();
        threadStop();
    }

    public void threadStop() {
        handler.removeCallbacks(this);
    }

    public boolean isPaused() {
        return timePaused != null;
    }

    public boolean isRunning() { return timeFinished != null;}

    @Override
    public void run()  {
        if((new Date()).after(timeFinished)) { // Are we done?
            del.timerFinished();
        } else {
            del.updateTime();
            handler.postDelayed(this, 500);
        }
    }
}
