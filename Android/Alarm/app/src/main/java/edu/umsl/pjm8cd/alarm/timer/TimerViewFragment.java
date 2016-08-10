package edu.umsl.pjm8cd.alarm.timer;

import android.app.AlarmManager;
import android.app.DatePickerDialog;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.TimePickerDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.DialogFragment;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.NotificationCompat;
import android.text.Layout;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import android.widget.TimePicker;

import java.util.Calendar;
import java.util.Date;
import java.util.UUID;

import edu.umsl.pjm8cd.alarm.AlarmActivityControler;
import edu.umsl.pjm8cd.alarm.R;
import edu.umsl.pjm8cd.alarm.alarm.Alarm;
import edu.umsl.pjm8cd.alarm.alarm.ListViewFragment;
import edu.umsl.pjm8cd.alarm.database.DBWrapper;

/**
 * Created by pat on 3/4/2016.
 *
 * view and controller Fragment to handle the timer. (One half of the tabs)
 */
public class TimerViewFragment extends Fragment
        implements Timer.TimerDelegate {

    Timer model;

    private TimePicker picker;

    private View pickerLayout;
    private View bodyLayout;
    private Button pauseResumeButton;

    private TextView timer;

    private PendingIntent pendingIntent;
    private AlarmManager alarmManager;
    public static TimerViewFragment newInstance() {
        TimerViewFragment fragment = new TimerViewFragment();
        return fragment;
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        setRetainInstance(true);//TODO
        View view = inflater.inflate(R.layout.timer_fragment_view, container, false);
        if(model == null) {
            model = new Timer(this);
        }

        picker = (TimePicker) view.findViewById(R.id.picker);
        pickerLayout = (View) view.findViewById(R.id.picker_body);
        bodyLayout = (View) view.findViewById(R.id.timer_body);
        timer = (TextView) view.findViewById(R.id.timer);

        final Button cancelButton = (Button) view.findViewById(R.id.reset_button);
        pauseResumeButton = (Button) view.findViewById(R.id.pause_button);
        final Button pickButton = (Button) view.findViewById(R.id.picker_button);

        // Create broadcast pending intent containing the UUID of the alarm
        Intent resultIntent = new Intent(getContext(), AlarmReceiver.class);
        pendingIntent = PendingIntent.getBroadcast(getContext(), 0, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        alarmManager = (AlarmManager) getActivity().getSystemService(Context.ALARM_SERVICE);


        pickButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                int hour, min;
                if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
                    hour = picker.getHour();
                    min = picker.getMinute();
                } else { // If version is before getHour() was implemented
                    Log.e("TIMEPICKER", "We don't actually support that version yet.");
                    return;
                }

                // Calculate how long the timer should run.
                Calendar currentTime = Calendar.getInstance();
                Calendar targetTime = Calendar.getInstance();
                targetTime.set(Calendar.HOUR_OF_DAY, hour);
                targetTime.set(Calendar.MINUTE, min);
                targetTime.set(Calendar.SECOND, 0);

                if(currentTime.after(targetTime)) { // If timer is set for tomorrow.
                    targetTime.add(Calendar.DATE, 1);
                }

                model.setTimeFinished(targetTime.getTime());// Set time in the model

                // Hide the picker and show the timer.
                pickerLayout.setVisibility(View.GONE);
                bodyLayout.setVisibility(View.VISIBLE);

                alarmManager.set(AlarmManager.RTC_WAKEUP, targetTime.getTime().getTime(), pendingIntent);

                updateTime();
                model.threadStart();
            }
        });

        pauseResumeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(model.isPaused()) {
                    model.threadStart();
                    alarmManager.set(AlarmManager.RTC_WAKEUP, model.getTimeFinished(), pendingIntent);
                    pauseResumeButton.setText("Pause");
                } else {
                    model.threadPause();
                    alarmManager.cancel(pendingIntent);
                    pauseResumeButton.setText("Resume");
                }
                updateTime();
            }
        });

        cancelButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                model.threadStop();
                alarmManager.cancel(pendingIntent);
                // Show the picker again on cancel.
                pickerLayout.setVisibility(View.VISIBLE);
                bodyLayout.setVisibility(View.GONE);
            }
        });

        // Resume state
        if(model.isRunning()) {
            pickerLayout.setVisibility(View.GONE);
            bodyLayout.setVisibility(View.VISIBLE);
            updateTime();
            if(model.isPaused()) {
                pauseResumeButton.setText("Resume");
            }
        }
        return view;
    }

    // Updates the text field to show the time remaining as specified in the model.
    public void updateTime() {
        timer.setText(model.getTimeString());
    }

    // Delegate method called when the timer has finished counting down.
    public void timerFinished() {
        pauseResumeButton.setVisibility(View.GONE);
        timer.setText("00:00:00");
    }

    @Override
    public void onPause() {
        super.onPause();
        model.threadStop();
    }

    @Override
    public void onResume() {
        super.onResume();
        model.threadTryStart();

    }

    // Receives an alarm when the timer has reached zero. Dispatches a notification.
    public static class AlarmReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            NotificationCompat.Builder builder = new NotificationCompat.Builder(context);
            builder.setSmallIcon(android.R.drawable.ic_dialog_alert);
            builder.setContentTitle("Timer Done");
            builder.setContentText("Your Timer has finished");

            Intent resultIntent = new Intent(context, AlarmActivityControler.class);
            // When the notification is selected the user should be directed to the timer screen.
            resultIntent.putExtra(AlarmActivityControler.OPEN_IN_TIMER, true);
            PendingIntent pendingIntent = PendingIntent.getActivity(context, 0, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);
            builder.setContentIntent(pendingIntent);

            int notificationId = 0;
            NotificationManager manager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
            manager.notify(notificationId, builder.build());

            // Play sound.
            MediaPlayer mp = MediaPlayer.create(context, R.raw.finished);
            mp.start();
        }
    }
}
