package ssu.btetris.singleuser;

import android.content.Intent;

import android.os.Bundle;
import android.view.View;
import android.widget.EditText;

import androidx.appcompat.app.AppCompatActivity;

public class SettingActivity extends AppCompatActivity {
    private EditText viewHostName;
    private EditText viewPortNo;
    private String servHostName;
    private String servPortNo;
    public void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_setting);
        viewHostName = (EditText) findViewById(R.id.editview1);
        viewPortNo = (EditText) findViewById(R.id.editview2);
        Intent reqmsg = getIntent();
        servHostName = reqmsg.getStringExtra("serverHostName");
        servPortNo = reqmsg.getStringExtra("serverPortNo");
        if (servHostName == null) {
            servHostName = "255.255.255.255";
            servPortNo = "9999";
        }
        viewHostName.setText(servHostName);
        viewPortNo.setText(servPortNo);
    }
    public void onClick(View v){
        switch(v.getId()) {
            case R.id.confirm:
                Intent resmsg = new Intent();
                resmsg.putExtra("servHostName", viewHostName.getText().toString());
                resmsg.putExtra("servPortNo", viewPortNo.getText().toString());
                setResult(RESULT_OK, resmsg);
                break;
            case R.id.cancel:
                setResult(RESULT_CANCELED);
                break;
        }
        finish(); // to self-terminate this Activity
    }
}
