using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ability : MonoBehaviour
{
    public float speed = 2f;
    public float distance = 0f;
    public float distanceMax = 0f;
    public float backspeed = 0.5f;

    public GameObject enemy;
    public GameObject wall;

    bool reset = false;
    bool rightUp = false;
    bool rightDown = false;
    bool leftUp = false;
    bool leftDown = false;
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;

    Vector3 back; 
    Vector3 temp;
    Vector3 temp2;

    void Start()
    {
        back = transform.localScale;
        speed = speed * 0.01f;
        backspeed = backspeed * 0.01f;
    }


    void Update()
    {
        if (Input.GetButtonDown("Fire1"))
        { reset = true; }


        if (transform.localScale.x <= distance && reset==true)
        {
            temp = transform.localScale;
            temp.x += Time.deltaTime + speed;
            temp.z += Time.deltaTime + speed;
            transform.localScale = temp;
        }
        else if (reset == true)
        {
            transform.localScale = back;
            reset = false;
            temp.x = 0f;
            temp.z = 0f;
        }


        if(enemy.transform.localPosition.x <= transform.localPosition.x + temp.x / 2 && enemy.transform.localPosition.z <= transform.localPosition.z + temp.z / 2 && enemy.transform.localPosition.x >= transform.localPosition.x && enemy.transform.localPosition.z >= transform.localPosition.z)
        {
            leftUp = true;
        }
        else if (enemy.transform.localPosition.x >= transform.localPosition.x - temp.x / 2 && enemy.transform.localPosition.z <= transform.localPosition.z + temp.z / 2 && enemy.transform.localPosition.x <= transform.localPosition.x && enemy.transform.localPosition.z >= transform.localPosition.z)
        {
            leftDown = true;
        }
        else if (enemy.transform.localPosition.x <= transform.localPosition.x + temp.x / 2 && enemy.transform.localPosition.z >= transform.localPosition.z - temp.z / 2 && enemy.transform.localPosition.x >= transform.localPosition.x && enemy.transform.localPosition.z <= transform.localPosition.z)
        {
            rightUp = true;
        }
        else if (enemy.transform.localPosition.x >= transform.localPosition.x - temp.x / 2 && enemy.transform.localPosition.z >= transform.localPosition.z - temp.z / 2 && enemy.transform.localPosition.x <= transform.localPosition.x && enemy.transform.localPosition.z <= transform.localPosition.z)
        {
            rightDown = true;
        }
      

            if (leftUp)
        {
            temp2 = enemy.transform.localPosition;
            temp2.x += Time.deltaTime + backspeed;
            temp2.z += Time.deltaTime + backspeed;
            enemy.transform.localPosition = temp2;
            if(enemy.transform.localPosition.x >= transform.localPosition.x + distanceMax || enemy.transform.localPosition.z >= transform.localPosition.z + distanceMax)
            {
                leftUp = false;
            }
        }
        else if(leftDown)
        {
            temp2 = enemy.transform.localPosition;
            temp2.x -= Time.deltaTime + backspeed;
            temp2.z += Time.deltaTime + backspeed;
            enemy.transform.localPosition = temp2;
            if (enemy.transform.localPosition.x <= transform.localPosition.x - distanceMax || enemy.transform.localPosition.z >= transform.localPosition.z + distanceMax)
            {
                leftDown = false;
            }
        }
        else if(rightUp)
        {
            temp2 = enemy.transform.localPosition;
            temp2.x += Time.deltaTime + backspeed;
            temp2.z -= Time.deltaTime + backspeed;
            enemy.transform.localPosition = temp2;
            if (enemy.transform.localPosition.x >= transform.localPosition.x + distanceMax || enemy.transform.localPosition.z <= transform.localPosition.z - distanceMax)
            {
                rightUp = false;
            }
        }
        else if(rightDown)
        {
            temp2 = enemy.transform.localPosition;
            temp2.x -= Time.deltaTime + backspeed;
            temp2.z -= Time.deltaTime + backspeed;
            enemy.transform.localPosition = temp2;
            if (enemy.transform.localPosition.x <= transform.localPosition.x - distanceMax || enemy.transform.localPosition.z <= transform.localPosition.z - distanceMax)
            {
                rightDown = false;
            }
        }
      
    }
}