package com.ffmpegdemo;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
//测试
public class MainActivity extends AppCompatActivity {


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        findViewById(R.id.bt_code).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startAct(DecoderActivity.class);
            }
        });
    }

    //
    private void startAct(Class startClass) {

        startActivity(new Intent(MainActivity.this, startClass));
    }
}
