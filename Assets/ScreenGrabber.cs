using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

public class ScreenGrabber : MonoBehaviour
{
    [DllImport("Plugin")]
    private static extern IntPtr GetRenderEventFunc();


    IEnumerator Start()
    {
        yield return StartCoroutine("CallPluginAtEndOfFrames");
    }

    private IEnumerator CallPluginAtEndOfFrames()
    {
        GL.IssuePluginEvent(GetRenderEventFunc(), 0);
        while (true)
        {
            // Wait until all frame rendering is done
            yield return new WaitForEndOfFrame();


            // Issue a plugin event with arbitrary integer identifier.
            // The plugin can distinguish between different
            // things it needs to do based on this ID.
            // For our simple plugin, it does not matter which ID we pass here.
            GL.IssuePluginEvent(GetRenderEventFunc(), 1);
            yield return new WaitForEndOfFrame();
            yield return new WaitForEndOfFrame();
            GL.IssuePluginEvent(GetRenderEventFunc(), 2);


        }
    }
}
