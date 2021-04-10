using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RandomMov : MonoBehaviour
{
    private bool selectNewRandomPosition = true;
    private bool waitingForNewPosition = false;
    public Transform startMarker;
    public Transform endMarker;
    public float minX = -5;
    public float maxX = 5;
    public float minY = 0;
    public float maxY = 3;
    public float minZ = -5;
    public float maxZ = 5;
    public float movementSpeed = 5;
    public float newPositionWaitTime = 0.2F;
    private float moveX;
    private float moveY;
    private float moveZ;
    private float newX;
    private float newY;
    private float newZ;
    private float stopX;
    private float stopY;
    private float stopZ;
    private float frameX;
    private float frameY;
    private float frameZ;
    private float movedX;
    private float movedY;
    private float movedZ;
    public float speed = 1.0F;
    private float startTime;
    private float journeyLength;

    void Update()
    {

        if (selectNewRandomPosition)
        {
            StartCoroutine(newRandomPosition());
        }
        else if (!waitingForNewPosition)
        {
            frameX = (moveX * Time.deltaTime * movementSpeed);
            frameY = (moveY * Time.deltaTime * movementSpeed);
            frameZ = (moveZ * Time.deltaTime * movementSpeed);
            movedX += frameX;
            movedY += frameY;
            movedZ += frameZ;
            newX = this.transform.position.x + frameX;
            newY = this.transform.position.y + frameY;
            newZ = this.transform.position.z + frameZ;
            if (Mathf.Abs(movedY) >= Mathf.Abs(moveX) || Mathf.Abs(movedY) >= Mathf.Abs(moveY) || Mathf.Abs(movedZ) >= Mathf.Abs(moveZ))
            {
                waitingForNewPosition = true;
                selectNewRandomPosition = true;
            }
            else
            {
                startTime = Time.time;


                journeyLength = Vector3.Distance(startMarker.position, endMarker.position);
               // journeyLength = Vector3(startMarker.position, endMarker.position);
                float distCovered = (Time.time - startTime) * speed;
                float fracJourney = distCovered / journeyLength;
                this.transform.position = Vector3.Lerp(startMarker.position, endMarker.position, fracJourney);
                this.transform.position = new Vector3(newX, newY, newZ);
                //this.transform.position = new Vector3(newX, newY, this.transform.position.z);
            }
        }
    }

    IEnumerator newRandomPosition()
    {
        waitingForNewPosition = true;
        selectNewRandomPosition = false;
        yield return new WaitForSeconds(newPositionWaitTime);
        moveX = Random.Range(minX, maxX);
        moveY = Random.Range(minY, maxY);
        moveZ = Random.Range(minZ, maxZ);
        stopX = this.transform.position.x + moveX;
        stopY = this.transform.position.y + moveY;
        stopZ = this.transform.position.z + moveZ;
        movedX = 0f;
        movedY = 0f;
        movedZ = 0f;
        waitingForNewPosition = false;
    }
}
