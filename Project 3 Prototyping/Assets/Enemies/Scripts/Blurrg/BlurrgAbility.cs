using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BlurrgAbility : MonoBehaviour
{
    private bool selectNewRandomPosition = true;
    private bool waitingForNewPosition = false;
    public Transform startMarker;
    public Transform endMarker; 
    public GameObject ally;
    public float minX;
    public float maxX;
    public float minY;
    public float maxY;
    public float minZ;
    public float maxZ;
    public float movementSpeed;
    public float newPositionWaitTime;
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
    public float distance;
    public float maxTimer;
    private float currentTimer = 0.0f;
    private float tackleSpeed;
    public GameObject target;
    bool up = true;
    bool start = true;

   
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        currentTimer += Time.deltaTime;
        if(Vector3.Distance(ally.transform.position, transform.position)<= distance)
        {
            if(currentTimer < maxTimer)
            {
                charge();
            }
            else
            {
                up = true;
                transform.position = new Vector3(transform.position.x, 0.9f, transform.position.z);
                tackle();
            }
        }
        else
        {
            up = true;
            transform.position = new Vector3 (transform.position.x, 0.9f, transform.position.z);
            currentTimer = 0f;
            RandomMov();
        }

    }
    void RandomMov()
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
    void charge()
    {
        if (up)
        {
          
            transform.Translate(0f, 1f, 0f);
            up = false;
        }
        else
        {
            transform.Translate(0f, -1f, 0f);
            up = true;
        }
        
    }
    void tackle()
    {
        if(start)
        {
            
            target.transform.position = ally.transform.position;
            start = false;
        }
        tackleSpeed = Time.deltaTime * movementSpeed * 5;
        transform.position = Vector3.MoveTowards(transform.position, target.transform.position, tackleSpeed);
        
        if(Vector3.Distance(transform.position, target.transform.position)<=0.45f)
        {
            currentTimer = 0f;
            start = true;
           
        }
        
        if(Vector3.Distance(transform.position, ally.transform.position) <= 1.3f)
        {
            currentTimer = 0f;
            start = true;
            print("DAMAGE");
        }
    }
}
