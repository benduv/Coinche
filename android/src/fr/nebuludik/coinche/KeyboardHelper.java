package fr.nebuludik.coinche;

import android.app.Activity;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.inputmethod.InputMethodManager;
import java.lang.reflect.Field;

public class KeyboardHelper {
    private static final int IME_FLAG_NO_EXTRACT_UI = 0x10000000;
    private static final int IME_FLAG_NO_FULLSCREEN = 0x02000000;

    public static void install(Activity activity) {
        View decorView = activity.getWindow().getDecorView();
        Handler handler = new Handler(Looper.getMainLooper());

        decorView.getViewTreeObserver().addOnGlobalFocusChangeListener(
            (oldFocus, newFocus) -> {
                if (newFocus != null && newFocus.getClass().getName().contains("QtEditText")) {
                    final View editText = newFocus;
                    // Delay to ensure Qt's setEditTextOptions has already run
                    handler.postDelayed(() -> patchImeOptions(editText), 50);
                }
            }
        );
    }

    private static void patchImeOptions(View editText) {
        try {
            Field f = editText.getClass().getDeclaredField("m_imeOptions");
            f.setAccessible(true);
            int current = f.getInt(editText);
            int patched = current | IME_FLAG_NO_EXTRACT_UI | IME_FLAG_NO_FULLSCREEN;
            if (current != patched) {
                f.setInt(editText, patched);
                // Force the IME to re-read the options
                InputMethodManager imm = (InputMethodManager)
                    editText.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
                if (imm != null) {
                    imm.restartInput(editText);
                }
            }
        } catch (Exception e) {
            // Silently fail on unsupported Qt versions
        }
    }
}
