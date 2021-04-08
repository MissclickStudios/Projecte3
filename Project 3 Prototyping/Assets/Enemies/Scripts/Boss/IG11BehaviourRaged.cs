using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class IG11BehaviourRaged : MonoBehaviour
{
    private bool trigerredAttackRaged;

    private bool selectedSpecialRaged;

    private bool readyTo360Attack;
    private bool readyToDoubleSpiralAttack;

    private float sAttack2FireRate;
    private float sAttack2Duration = 10.0f;

    private float _360AttackFireRate;
    private float _360AttackDuration = 5.0f;

    private float doubleSpiralAttackFireRate;
    private float doubleSpiralAttackDuration = 5.0f;
    // Start is called before the first frame update
    void Start()
    {
        trigerredAttackRaged = false;

        moveRight = true;

        readyTo360Attack = false;
        readyToDoubleSpiralAttack = false;
        selectedSpecialRaged = false; //False for 360, true for spiral
    }

    // Update is called once per frame
    void Update()
    {
        if (readyTo360Attack == false || readyToDoubleSpiralAttack == false)
        {
            sAttack2Duration -= Time.deltaTime;
            Chasing();
            if (trigerredAttackRaged == true)
            {
                standardAttackNR();
            }
            if (sAttack2Duration < 0.0f)
            {
                if (selectedSpecialRaged == false)
                {
                    coneAttackNRPrep();
                }

                else if (selectedSpecialRaged == true)
                {
                    spiralAttackNRPrep();
                }
            }
        }

        if (readyTo360Attack == true)
        {
            coneAttackNRMov();

            _360AttackFireRate += Time.deltaTime;
            _360AttackDuration -= Time.deltaTime;

            if (_360AttackFireRate >= 0.5f)
            {
                startPoint = transform.position;
                coneAttackNR(SA1numProjectiles);
                _360AttackFireRate = 0;
            }
            if (_360AttackDuration < 0.0f)
            {
                readyTo360Attack = false;
                _360AttackDuration = 5.0f;
                selectedSpecialRaged = true;
                sAttack2Duration = 10.0f;
            }
        }

        if (readyToDoubleSpiralAttack == true)
        {

            spiralAttackNRMov();

            doubleSpiralAttackFireRate += Time.deltaTime;
            doubleSpiralAttackDuration -= Time.deltaTime;

            if (doubleSpiralAttackFireRate >= 0.05f)
            {
                startPoint = transform.position;
                spiralAttackNR();
                doubleSpiralAttackFireRate = 0;
            }
            if (doubleSpiralAttackDuration < 0.0f)
            {
                readyToDoubleSpiralAttack = false;
                doubleSpiralAttackDuration = 5.0f;
                selectedSpecialRaged = false;
                sAttack2Duration = 10.0f;
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

        if ((transform.position - chasingTarget.position).magnitude > chasingTriggerDistance)
        {

            transform.Translate(0.0f, 0.0f, chasingSpeed * Time.deltaTime);

        }
        else if ((transform.position - chasingTarget.position).magnitude < chasingTriggerDistance && trigerredAttackRaged == false)
        {

            trigerredAttackRaged = true;

        }
    }
    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------
    [Header("Standard Attack Raged Settings")]
    public GameObject sAttack1Bullet;
    public Transform sAttack1Target;
    public Transform sAttack1RightArm;
    public Transform sAttack1LeftArm;
    public Transform sAttack1Head;
    public float sAttack1BulletSpeed = 100;

    void standardAttackNR()
    {
        sAttack2FireRate += Time.deltaTime;

        if (sAttack2FireRate >= 0.5f)
        {
            GameObject myBulletPrefab = Instantiate(sAttack1Bullet, sAttack1RightArm.position, Quaternion.identity) as GameObject;
            Rigidbody myBulletPrefabRigidBody = myBulletPrefab.GetComponent<Rigidbody>();
            myBulletPrefabRigidBody.AddForce(transform.forward * sAttack1BulletSpeed);

            GameObject myBulletPrefab2 = Instantiate(sAttack1Bullet, sAttack1LeftArm.position, Quaternion.identity) as GameObject;
            Rigidbody myBulletPrefabRigidBody2 = myBulletPrefab2.GetComponent<Rigidbody>();
            myBulletPrefabRigidBody2.AddForce(transform.forward * sAttack1BulletSpeed);

            GameObject myBulletPrefab3 = Instantiate(sAttack1Bullet, sAttack1Head.position, Quaternion.identity) as GameObject;
            Rigidbody myBulletPrefabRigidBody3 = myBulletPrefab3.GetComponent<Rigidbody>();
            myBulletPrefabRigidBody3.AddForce(transform.forward * sAttack1BulletSpeed);

            sAttack2FireRate = 0;
            trigerredAttackRaged = false;
        }
    }
    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------
    [Header("Preparing 360 Attack Raged Settings")]
    public Transform Target;
    public Transform Boss;
    public float Speed = 1.0f;

    void coneAttackNRPrep()
    {
        Boss.transform.position = Vector3.MoveTowards(Boss.transform.position, Target.transform.position, Speed * Time.deltaTime);

        if (Boss.transform.position == Target.transform.position)
        {
            readyTo360Attack = true;
        }
    }

    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------

    [Header("Movement 360 Attack Raged Settings")]
    public float moveSpeed = 1.0f;
    private bool moveRight;
    void coneAttackNRMov()
    {
        //ROTATE ON Y AXIS
        transform.Rotate(Vector3.up * rotateSpeed * Time.deltaTime);
    }
    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------
    [Header("Special Attack 1 Raged Settings")]
    public int SA1numProjectiles;
    public float SA1projectileSpeed;
    public GameObject SA1projectile;

    private Vector3 startPoint;
    private const float radius = 1F;

    void coneAttackNR(int _SA1numProjectiles)
    {
        float angleStep = 360f / _SA1numProjectiles;
        float angle = 0f;

        for (int i = 1; i <= _SA1numProjectiles * 2; i++)
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
    [Header("Preparing Spiral Attack Raged Settings")]
    public Transform spiralTarget;
    public Transform spiralBoss;
    public float spiralSpeed = 1.0f;

    void spiralAttackNRPrep()
    {
        spiralBoss.transform.position = Vector3.MoveTowards(spiralBoss.transform.position, spiralTarget.transform.position, spiralSpeed * Time.deltaTime);

        if (spiralBoss.transform.position == spiralTarget.transform.position)
        {
            readyToDoubleSpiralAttack = true;
        }
    }

    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------

    [Header("Movement Spiral Attack Raged Settings")]
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
    [Header("Special Attack 2 Raged Settings")]
    public GameObject SA2ProjectilePrefab;
    public float SA2projectileSpeed;

    private float angle = 0f;

    void spiralAttackNR()
    {
        for (int i = 0; i <= 1; i++)
        {
            // Direction Calculation

            float projectileDirXPosition = startPoint.x + Mathf.Sin(((angle + 180f * i) * Mathf.PI) / 180f);
            float projectileDirYPosition = startPoint.y + Mathf.Cos(((angle + 180f * i) * Mathf.PI) / 180f);

            Vector3 projectileVector = new Vector3(projectileDirXPosition, projectileDirYPosition, 0);
            Vector3 projectileMoveDirection = (projectileVector - startPoint).normalized * SA2projectileSpeed;

            GameObject tmpObj = Instantiate(SA2ProjectilePrefab, startPoint, Quaternion.identity);
            tmpObj.GetComponent<Rigidbody>().velocity = new Vector3(projectileMoveDirection.x, 0, projectileMoveDirection.y);

        }

        angle += 10f;

        if (angle >= 360f)
        {
            angle = 0f;
        }
    }

    //---------------------------------------------------------------------------------
}
