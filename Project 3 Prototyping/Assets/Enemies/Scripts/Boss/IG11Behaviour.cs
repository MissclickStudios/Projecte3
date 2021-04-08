using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class IG11Behaviour : MonoBehaviour
{
    private bool trigerredAttack;

    private bool selectedSpecial;

    private bool lookat;

    private bool readyToConeAttack;
    private bool readyToSpiralAttack;

    private float sAttack1FireRate;
    private float sAttack1Duration = 10.0f;

    private float coneAttackFireRate;
    private float coneAttackDuration = 5.0f;

    private float spiralAttackFireRate;
    private float spiralAttackDuration = 5.0f;
    // Start is called before the first frame update
    void Start()
    {
        trigerredAttack = false;

        moveRight = true;
        lookat = true;

        readyToConeAttack = false;
        readyToSpiralAttack = false;
        selectedSpecial = false; //False for cone, true for spiral
    }

    // Update is called once per frame
    void Update()
    {
        if(readyToConeAttack == false || readyToSpiralAttack == false)
        {
            sAttack1Duration -= Time.deltaTime;
            Chasing();
            if (trigerredAttack == true)
            {
                standardAttackNR();
            }
            if (sAttack1Duration < 0.0f)
            {
                lookat = false;

                if (selectedSpecial == false)
                {
                    coneAttackNRPrep();
                }

                else if (selectedSpecial == true)
                {
                    spiralAttackNRPrep();
                }
            }
        }

        if (readyToConeAttack == true)
        {

            coneAttackNRMov();

            coneAttackFireRate += Time.deltaTime;
            coneAttackDuration -= Time.deltaTime;

            if (coneAttackFireRate >= 1.0f)
            {
                startPoint = transform.position;
                coneAttackNR(SA1numProjectiles);
                coneAttackFireRate = 0;
            }
            if (coneAttackDuration < 0.0f)
            {
                readyToConeAttack = false;
                coneAttackDuration = 5.0f;
                selectedSpecial = true;
                sAttack1Duration = 10.0f;
                lookat = true;
            }
        }

        if (readyToSpiralAttack == true)
        {

            spiralAttackNRMov();

            spiralAttackFireRate += Time.deltaTime;
            spiralAttackDuration -= Time.deltaTime;

            if (spiralAttackFireRate >= 0.05f)
            {
                startPoint = transform.position;
                spiralAttackNR();
                spiralAttackFireRate = 0;
            }
            if (spiralAttackDuration < 0.0f)
            {
                readyToSpiralAttack = false;
                spiralAttackDuration = 5.0f;
                selectedSpecial = false;
                sAttack1Duration = 10.0f;
                lookat = true;
            }
        }

    }

    //---------------------------------------------------------------------------------
    [Header("Chasing Settings")]
    public Transform chasingTarget;
    public float chasingSpeed = 1.0f;
    public float chasingTriggerDistance = 5.0f;

    void Chasing()
    {
        if (lookat == true)
        {
            transform.LookAt(chasingTarget.position);
        }

        if ((transform.position - chasingTarget.position).magnitude > chasingTriggerDistance )
        {

            transform.Translate(0.0f, 0.0f, chasingSpeed * Time.deltaTime);
            
        }
        else if ((transform.position - chasingTarget.position).magnitude < chasingTriggerDistance && trigerredAttack == false)
        {

            trigerredAttack = true;

        }
    }
    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------
    [Header("Standard Attack Not Raged Settings")]
    public GameObject sAttack1Bullet;
    public Transform sAttack1Target;
    public Transform sAttack1RightArm;
    public Transform sAttack1LeftArm;
    public float sAttack1BulletSpeed = 100;

    void standardAttackNR()
    {
        sAttack1FireRate += Time.deltaTime;

        if (sAttack1FireRate >= 0.5f)
        {
            GameObject myBulletPrefab = Instantiate(sAttack1Bullet, sAttack1RightArm.position, Quaternion.identity) as GameObject;
            Rigidbody myBulletPrefabRigidBody = myBulletPrefab.GetComponent<Rigidbody>();
            myBulletPrefabRigidBody.AddForce(transform.forward * sAttack1BulletSpeed);

            GameObject myBulletPrefab2 = Instantiate(sAttack1Bullet, sAttack1LeftArm.position, Quaternion.identity) as GameObject;
            Rigidbody myBulletPrefabRigidBody2 = myBulletPrefab2.GetComponent<Rigidbody>();
            myBulletPrefabRigidBody2.AddForce(transform.forward * sAttack1BulletSpeed);

            sAttack1FireRate = 0;
            trigerredAttack = false;
        }
    }
    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------
    [Header("Preparing Cone Attack Not Raged Settings")]
    public Transform Target;
    public Transform Boss;
    public float Speed = 1.0f;
   
    void coneAttackNRPrep()
    {
        Boss.transform.position = Vector3.MoveTowards(Boss.transform.position, Target.transform.position, Speed * Time.deltaTime);

        if(Boss.transform.position == Target.transform.position)
        {
            readyToConeAttack = true;
        }
    }

    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------

    [Header("Movement Cone Attack Not Raged Settings")]
    public float moveSpeed = 1.0f;
    private bool moveRight;
    void coneAttackNRMov()
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
    }
    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------
    [Header("Special Attack 1 Not Raged Settings")]
    public int SA1numProjectiles;
    public float SA1projectileSpeed;
    public GameObject SA1projectile;

    private Vector3 startPoint;
    private const float radius = 1F;

    void coneAttackNR(int _SA1numProjectiles)
    {
        float startAngle = 120f, endAngle = 240f;
        float angleStep = (endAngle - startAngle)/ _SA1numProjectiles;
        float angle = startAngle;

        for (int i = 1; i <= _SA1numProjectiles + 1; i++)
        {
            // Direction Calculation

            float projectileDirXPosition = startPoint.x + Mathf.Sin((angle * Mathf.PI) / 180) * radius;
            float projectileDirYPosition = startPoint.y + Mathf.Cos((angle * Mathf.PI) / 180) * radius;

            Vector3 projectileVector = new Vector3(projectileDirXPosition, projectileDirYPosition, 0);
            Vector3 projectileMoveDirection = (projectileVector - startPoint).normalized * SA1projectileSpeed;

            GameObject tmpObj = Instantiate(SA1projectile, startPoint, Quaternion.identity);
            tmpObj.GetComponent<Rigidbody>().velocity = new Vector3(projectileMoveDirection.x, 0, projectileMoveDirection.y);

            angle += angleStep;
        }
    }

    //---------------------------------------------------------------------------------
    [Header("Preparing Spiral Attack Not Raged Settings")]
    public Transform spiralTarget;
    public Transform spiralBoss;
    public float spiralSpeed = 1.0f;

    void spiralAttackNRPrep()
    {
        spiralBoss.transform.position = Vector3.MoveTowards(spiralBoss.transform.position, spiralTarget.transform.position, spiralSpeed * Time.deltaTime);

        if (spiralBoss.transform.position == spiralTarget.transform.position)
        {
            readyToSpiralAttack = true;
        }
    }

    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------

    [Header("Movement Spiral Attack Not Raged Settings")]
    public float rotateSpeed = 100;

    private float timeCounter = 0;
    public float circleSpeed = 5;
    public float circleWidth = 4;
    public float circleLength = 8;
    void spiralAttackNRMov()
    {
        //MOVE ON CIRCLES
        timeCounter += Time.deltaTime * circleSpeed;

        float x = Mathf.Sin(timeCounter) + circleWidth;
        float y = 1.46f;
        float z = Mathf.Cos(timeCounter) + circleLength; ;

        transform.position = new Vector3(x, y, z);

        //ROTATE ON Y AXIS
        transform.Rotate(Vector3.up * rotateSpeed * Time.deltaTime);
    }
    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------
    [Header("Special Attack 2 Not Raged Settings")]
    public GameObject SA2ProjectilePrefab;
    public float SA2projectileSpeed;

    private float angle = 0f;

    private float dt;
    void spiralAttackNR()
    {
        float projectileDirXPosition = startPoint.x + Mathf.Sin((angle * Mathf.PI) / 180);
        float projectileDirYPosition = startPoint.y + Mathf.Cos((angle * Mathf.PI) / 180);

        Vector3 projectileVector = new Vector3(projectileDirXPosition, projectileDirYPosition, 0);
        Vector3 projectileMoveDirection = (projectileVector - startPoint).normalized * SA2projectileSpeed;

        GameObject tmpObj = Instantiate(SA2ProjectilePrefab, startPoint, Quaternion.identity);
        tmpObj.GetComponent<Rigidbody>().velocity = new Vector3(projectileMoveDirection.x, 0, projectileMoveDirection.y);

        angle += 10;
    }
   
    //---------------------------------------------------------------------------------
}
