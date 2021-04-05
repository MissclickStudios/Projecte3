using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class IG11Behaviour : MonoBehaviour
{
    private bool trigerredAttack;

    private bool readyToConeAttack;

    private float sAttack1FireRate;
    private float sAttack1Duration = 10.0f;
    private float coneAttackFireRate;
    private float coneAttackDuration = 5.0f;

    // Start is called before the first frame update
    void Start()
    {
        trigerredAttack = false;
        readyToConeAttack = false;
    }

    // Update is called once per frame
    void Update()
    {
        if(readyToConeAttack == false)
        {
            sAttack1Duration -= Time.deltaTime;
            Chasing();
            if (trigerredAttack == true)
            {
                standardAttackNR();
            }
            if (sAttack1Duration < 0.0f)
            {
                coneAttackNRPrep();
            }
        }
       


        if (readyToConeAttack == true)
        {
            sAttack1Duration = 10.0f;

            coneAttackNRMov();

            coneAttackFireRate += Time.deltaTime;
            coneAttackDuration -= Time.deltaTime;

            if (coneAttackFireRate >= 1.0f)
            {
                startPoint = transform.position;
                coneAttackNR(SA1numProjectiles);
                coneAttackFireRate = 0;
            }
            if(coneAttackDuration < 0.0f)
            {
                readyToConeAttack = false;
                coneAttackDuration = 5.0f;
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
        transform.LookAt(chasingTarget.position);

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
}
