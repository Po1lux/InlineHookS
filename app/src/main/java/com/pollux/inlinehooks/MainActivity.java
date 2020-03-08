package com.pollux.inlinehooks;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.io.DataOutputStream;

public class MainActivity extends AppCompatActivity {
    private final static String iTAG = "cs";
    private TextView tv0;
    private Button bt0;
    private Button bt1;
    private Button bt2_treger;
    private Button bt3_ptrace;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        String apkRoot = "chmod 777 " + getPackageCodePath();
        SystemManager.RootCommand(apkRoot);

        tv0 = findViewById(R.id.tv0);
        bt0 = findViewById(R.id.bt0);
        bt1 = findViewById(R.id.bt1);
        bt2_treger =findViewById(R.id.bt2_tregethookRet);
        bt3_ptrace = findViewById(R.id.bt3_ptrace);
        bt0.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                updateText();
            }
        });


        //加载interface库，即开始hook
        bt1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                System.loadLibrary("interface");
                //Log.i("cs","System.loadLibrary(\"interface\");");
            }
        });

        //触发函数，测试hook函数返回值是否成功
        bt2_treger.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                hookRet();
            }
        });

        bt3_ptrace.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ptracetest();
            }
        });



    }

    private void updateText(){
        tv0.postDelayed(new Runnable() {
            @Override
            public void run() {
                tv0.setText(getTikNum());
                updateText();
            }
        },1000);
    }

    static{
        System.loadLibrary("native");
    }
    private static native String getTikNum();
    private static native void hookRet();

    private static native void ptracetest();



}
