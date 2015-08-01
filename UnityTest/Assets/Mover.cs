using UnityEngine;
using System.Collections;

public class Mover : MonoBehaviour {
	void Update () {
		transform.position += Vector3.right * Time.deltaTime * 3f;
	}
}
