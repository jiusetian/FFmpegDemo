package com.ffmpegdemo;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

public class DecoderActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_decoder);

        Button startButton = (Button) this.findViewById(R.id.bt_start);
        final EditText urlEdittext_input= (EditText) this.findViewById(R.id.et01);
        final EditText urlEdittext_output= (EditText) this.findViewById(R.id.editText2);

        startButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View arg0){

                String folderurl= Environment.getExternalStorageDirectory().getPath()+"/DCIM/Camera";

                String urltext_input=urlEdittext_input.getText().toString();
                final String inputurl=folderurl+"/"+urltext_input;

                String urltext_output=urlEdittext_output.getText().toString();
                final String outputurl=folderurl+"/"+urltext_output;
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        AvUtil.decoder(inputurl,outputurl);
                    }
                }).start();

            }
        });

    }
}
















