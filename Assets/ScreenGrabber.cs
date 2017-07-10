using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

public class ScreenGrabber : MonoBehaviour
{
    [DllImport("Plugin")]
    static extern int Foo();

    [DllImport("Plugin")]
    private static extern IntPtr GetRenderEventFunc();

    [DllImport("Plugin")]
    private static extern IntPtr MakeGrabTexture(uint width, uint height);


    Texture2D tex;

    IEnumerator Start()
    {
        //tex = new Texture2D(Screen.width, Screen.height);

        IntPtr p = MakeGrabTexture((uint)Screen.width, (uint)Screen.height);
        tex = Texture2D.CreateExternalTexture(Screen.width, Screen.height, TextureFormat.RGBA32, false, true, p);

        yield return StartCoroutine("CallPluginAtEndOfFrames");
    }

    private IEnumerator CallPluginAtEndOfFrames()
    {
        while (true)
        {
            // Wait until all frame rendering is done
            yield return new WaitForEndOfFrame();

            tex.ReadPixels(new Rect(0, 0, Screen.width, Screen.height), 0, 0);
            tex.Apply();

            // Issue a plugin event with arbitrary integer identifier.
            // The plugin can distinguish between different
            // things it needs to do based on this ID.
            // For our simple plugin, it does not matter which ID we pass here.
            GL.IssuePluginEvent(GetRenderEventFunc(), 1);
            yield return new WaitForEndOfFrame();
            yield return new WaitForEndOfFrame();
            GL.IssuePluginEvent(GetRenderEventFunc(), 2);
            var x = tex.GetRawTextureData();

            File.WriteAllBytes(@"C:\Users\brab\meep", x);

            break;

        }
    }
}
