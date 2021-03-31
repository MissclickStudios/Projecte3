using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BossMovementSpecial1 : MonoBehaviour
{
    private float moveSpeed;
    private bool moveRight;

    public float rotateSpeed = 100;

    private float timeCounter = 0;
    public float circleSpeed = 5;
    public float circleWidth = 4;
    public float circleLength = 8;

    // Start is called before the first frame update
    void Start()
    {
        moveSpeed = 10f;
        moveRight = true;
    }

    // Update is called once per frame
    void Update()
    {
        // LEFT TO RIGHT
        if (transform.position.x > 12f)
        {
            moveRight = false;
        }

        else if (transform.position.x < -12f)
        {
            moveRight = true;
        }

        if (moveRight)
        {
            transform.position = new Vector3(transform.position.x + moveSpeed * Time.deltaTime, transform.position.y, transform.position.z);
        }
        else
        {
            transform.position = new Vector3(transform.position.x - moveSpeed * Time.deltaTime, transform.position.y, transform.position.z);
        }

        //MOVE ON CIRCLES
        //timeCounter += Time.deltaTime*circleSpeed;

        //float x = Mathf.Sin (timeCounter)+circleWidth;
        //float y = 1.46f;
        //float z = Mathf.Cos(timeCounter) + circleLength; ;

        //transform.position = new Vector3(x, y, z);

        //ROTATE ON Y AXIS
        transform.Rotate(Vector3.up * rotateSpeed * Time.deltaTime);
    }
}
