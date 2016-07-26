 using UnityEngine;
using System.Collections;

public class lightsaber : MonoBehaviour {

	LineRenderer lineRend;
	public Transform startPos;
	public Transform endPos;
	private float textureOffset = 0f;
	private bool on=true;
	private Vector3 endPosExtendedPos;

	// Use this for initialization
	void Start () {
		lineRend = GetComponent<LineRenderer>();
		endPosExtendedPos = endPos.localPosition;
	}
	
	// Update is called once per frame
	void Update () {
		if (Input.GetKeyDown(KeyCode.Space)) {
			on = !on;
		}
		if (on) {
			endPos.localPosition = Vector3.Lerp (endPos.localPosition, endPosExtendedPos, Time.deltaTime * 5);
		} else {
			endPos.localPosition = Vector3.Lerp (endPos.localPosition, startPos.localPosition, Time.deltaTime * 5);
		} 
		lineRend.SetPosition (0, startPos.position);
		lineRend.SetPosition (1, endPos.position);
		textureOffset -= Time.deltaTime * 2f;
		if (textureOffset < -10f) {
			textureOffset += 10f;
		}
		lineRend.sharedMaterials[1].SetTextureOffset ("_MainTex", new Vector2(textureOffset,0f));
		if (Input.GetKeyDown(KeyCode.LeftArrow))
		{
			Vector3 position = this.transform.position;
			position.x--;
			this.transform.position = position;
		}
		if (Input.GetKeyDown(KeyCode.RightArrow))
		{
			Vector3 position = this.transform.position;
			position.x++;
			this.transform.position = position;
		}
		if (Input.GetKeyDown(KeyCode.UpArrow))
		{
			Vector3 position = this.transform.position;
			position.y++;
			this.transform.position = position;
		}
		if (Input.GetKeyDown(KeyCode.DownArrow))
		{
			Vector3 position = this.transform.position;
			position.y--;
			this.transform.position = position;
		}

	}
}