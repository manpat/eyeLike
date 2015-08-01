using UnityEngine;
using System.Collections;

public class Test : MonoBehaviour {
	EyeTracker et;

	void Start(){
		et = GetComponent<EyeTracker>();
	}

	void Update () {
		var p = transform.position;
		p.y = (float)et.data * 3f;
		transform.position = p;
	}
}
