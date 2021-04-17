using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ATSTBehaviour : MonoBehaviour
{
    private bool trigerredAttack;

    private bool readyPointAttack;
    private bool readyWaveAttack;
    private bool readyGranadeAttack;

    private bool chasing;

    private float sAttack1FireRate;
    private float sAttack1Duration = 10.0f;

    private float pointAttackFireRate;
    private float pointAttackDuration = 10.0f;

    private float waveAttackFireRate;
    private float waveAttackDuration = 10.0f;

    void Start()
    {
        trigerredAttack = false;

        moveRight = true;
        waveMoveRight = true;

        chasing = true;

        readyPointAttack = false;
        readyWaveAttack = false;

    }

    // Update is called once per frame
    void Update()
    {

        sAttack1Duration -= Time.deltaTime;
        if (chasing == true)
        {
            Chasing();
        }

        if (trigerredAttack == true)
        {
            standardAttack();
        }
        if (sAttack1Duration < 0.0f)
        {
            chasing = false;
            pointAttackPrep();
        }

        if (readyPointAttack == true)
        {
            //pointAttackMov();

            pointAttackFireRate += Time.deltaTime;
            pointAttackDuration -= Time.deltaTime;

            if (pointAttackFireRate >= 0.1f)
            {
                startPoint = shootSource.transform.position;
                pointAttack();
                pointAttackFireRate = 0;
            }

            if (pointAttackDuration < 0.0f)
            {

                readyPointAttack = false;

                waveAttackDuration = 10.0f;
                sAttack1Duration = 10.0f;

                waveAttackPrep();
            }
        }

        if (readyWaveAttack == true)
        {
            waveAttackMov();

            waveAttackFireRate += Time.deltaTime;
            waveAttackDuration -= Time.deltaTime;

            if (waveAttackFireRate >= 0.6f)
            {
                startPoint = waveShootSource.transform.position;
                waveAttack(waveNumProjectiles);
                waveAttackFireRate = 0;
            }

            if (waveAttackDuration < 0.0f)
            {
                readyWaveAttack = false;

                pointAttackDuration = 10.0f;

                grenadeAttackPrep();
            }
        }

        if (readyGranadeAttack == true)
        {

        }
    }

    //---------------------------------------------------------------------------------
    [Header("Chasing Settings")]
    public Transform chasingTarget;
    public Transform rotatoryPiece;
    public Transform legs;
    public float chasingSpeed = 1.0f;
    public float chasingTriggerDistance = 5.0f;
    void Chasing()
    {

        rotatoryPiece.transform.LookAt(chasingTarget.position);

        if ((transform.position - chasingTarget.position).magnitude > chasingTriggerDistance)
        {

            transform.Translate(0.0f, 0.0f, chasingSpeed * Time.deltaTime);

        }
        else if ((transform.position - chasingTarget.position).magnitude < chasingTriggerDistance && trigerredAttack == false)
        {

            trigerredAttack = true;

        }
    }
    //---------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------
    [Header("Standard Attack Settings")]
    public GameObject sAttack1Bullet;
    public Transform sAttack1RightArm;
    public Transform sAttack1LeftArm;
    public float rpm = 0.5f;
    public float sAttack1BulletSpeed = 100;
    void standardAttack()
    {
        sAttack1FireRate += Time.deltaTime;

        if (sAttack1FireRate >= rpm)
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
    [Header("Preparing Wave Attack Settings")]
    public Transform Target;
    public Transform Boss;
    public float Speed = 1.0f;

    void pointAttackPrep()
    {
        Boss.transform.position = Target.transform.position;

        if (Boss.transform.position == Target.transform.position)
        {
            readyPointAttack = true;
        }
    }
    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------
    [Header("Point Move Settings")]
    public float moveSpeed = 1.0f;
    private bool moveRight;
    void pointAttackMov()
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
    [Header("Point Attack Settings")]
    public GameObject pointAttack1Bullet;
    public Transform shootSource;
    public Transform pointAttack1RightArm;
    public Transform pointAttack1LeftArm;
    public float pointAttack1BulletSpeed = 100;

    void pointAttack()
    {
        rotatoryPiece.transform.LookAt(chasingTarget.position);

        GameObject myBulletPrefab = Instantiate(pointAttack1Bullet, pointAttack1RightArm.position, Quaternion.identity) as GameObject;
        Rigidbody myBulletPrefabRigidBody = myBulletPrefab.GetComponent<Rigidbody>();
        myBulletPrefabRigidBody.AddForce(transform.forward * pointAttack1BulletSpeed);

        GameObject myBulletPrefab2 = Instantiate(pointAttack1Bullet, pointAttack1LeftArm.position, Quaternion.identity) as GameObject;
        Rigidbody myBulletPrefabRigidBody2 = myBulletPrefab2.GetComponent<Rigidbody>();
        myBulletPrefabRigidBody2.AddForce(transform.forward * pointAttack1BulletSpeed);

        sAttack1FireRate = 0;
    }
    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------
    [Header("Preparing Wave Attack Settings")]
    public Transform waveTarget;
    public Transform waveBoss;

    void waveAttackPrep()
    {
        waveBoss.transform.position = waveTarget.transform.position; 

        if (waveBoss.transform.position == waveTarget.transform.position)
        {
            readyWaveAttack = true;
        }
    }
    //---------------------------------------------------------------------------------


    //---------------------------------------------------------------------------------
    [Header("Wave Move Settings")]
    public float waveMoveSpeed = 1.0f;
    private bool waveMoveRight;
    void waveAttackMov()
    {
        // LEFT TO RIGHT
        if (transform.position.x > 12f)
        {
            waveMoveRight = false;
        }

        else if (transform.position.x < -12f)
        {
            waveMoveRight = true;
        }

        if (waveMoveRight)
        {
            transform.position = new Vector3(transform.position.x + waveMoveSpeed * Time.deltaTime, transform.position.y, transform.position.z);
        }
        else
        {
            transform.position = new Vector3(transform.position.x - waveMoveSpeed * Time.deltaTime, transform.position.y, transform.position.z);
        }
    }
    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------
    [Header("Wave Attack Settings")]
    public int waveNumProjectiles;
    public float waveProjectileSpeed;
    public GameObject waveProjectile;
    public Transform waveShootSource;

    private const float waveRadius = 1F;
    private Vector3 startPoint;

    void waveAttack(int _waveNumProjectiles)
    {
        float startAngle = 200f, endAngle = 170f;
        float angleStep = (endAngle - startAngle) / _waveNumProjectiles;
        float angle = startAngle;

        for (int i = 1; i <= _waveNumProjectiles + 1; i++)
        {
            // Direction Calculation

            float projectileDirXPosition = startPoint.x + Mathf.Sin((angle * Mathf.PI) / 180) * waveRadius;
            float projectileDirYPosition = startPoint.y + Mathf.Cos((angle * Mathf.PI) / 180) * waveRadius;

            Vector3 projectileVector = new Vector3(projectileDirXPosition, projectileDirYPosition, 0);
            Vector3 projectileMoveDirection = (projectileVector - startPoint).normalized * waveProjectileSpeed;

            GameObject tmpObj = Instantiate(waveProjectile, startPoint, Quaternion.identity);
            tmpObj.GetComponent<Rigidbody>().velocity = new Vector3(projectileMoveDirection.x, 0, projectileMoveDirection.y);

            angle += angleStep;
        }
    }
    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------
    [Header("Preparing Grenade Attack Settings")]
    public Transform grenadeTarget;
    public Transform grenadeBoss;

    void grenadeAttackPrep()
    {
        grenadeBoss.transform.position = grenadeTarget.transform.position;

        if (grenadeBoss.transform.position == grenadeTarget.transform.position)
        {
            readyGranadeAttack = true;
        }
    }
    //---------------------------------------------------------------------------------

}



