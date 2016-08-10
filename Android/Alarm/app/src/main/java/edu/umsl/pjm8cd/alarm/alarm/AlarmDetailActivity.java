package edu.umsl.pjm8cd.alarm.alarm;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.FragmentManager;
import android.support.v7.app.AppCompatActivity;

import edu.umsl.pjm8cd.alarm.R;

/**
 * Created by Pat on 5/1/2016.
 *
 * Activity for the AlarmDetailsView fragment.
 */
public class AlarmDetailActivity extends AppCompatActivity {
    private AlarmDetailsView alarmDetailsView;

    public static final String UUID = "UUID";

    public static Intent newIntent(Context currentContext) {
        Intent i = new Intent(currentContext, AlarmDetailActivity.class);
        return i;
    }

    /* Brings up the specified alarm for editing. */
    public static Intent newIntent(Context currentContext, Alarm timer) {
        Intent i = AlarmDetailActivity.newIntent(currentContext);
        i.putExtra(UUID, timer.getId().toString());
        return i;
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.blank_layout);

        FragmentManager manager = getSupportFragmentManager();
        alarmDetailsView = (AlarmDetailsView) manager.findFragmentById(R.id.fragment_detail_holder);

        if (alarmDetailsView == null) {
            alarmDetailsView = new AlarmDetailsView();
            manager.beginTransaction()
                    .add(R.id.fragment_detail_holder, alarmDetailsView)
                    .commit();
        }
    }
}