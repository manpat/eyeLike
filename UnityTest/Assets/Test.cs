using UnityEngine;
using System.Collections;

public class Test : MonoBehaviour {
	EyeTracker et;

    public Material matRed;
    public Material matGreen;

	void Start(){
		et = GetComponent<EyeTracker>();
	}

	void Update () {
		var p = transform.position;
		p.y = (float)et.adjustedData * 3f;
		transform.position = p;

        if (et.isLooking)
            renderer.sharedMaterial = matGreen;
        else
            renderer.sharedMaterial = matRed;
	}
}
