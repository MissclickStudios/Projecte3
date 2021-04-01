using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class IG11Behaviour : MonoBehaviour
{
    private bool trigerredAttack;

    // Start is called before the first frame update
    void Start()
    {
        trigerredAttack = false;
    }

    // Update is called once per frame
    void Update()
    {
        Chasing();
        if(trigerredAttack == true)
        {
            standardAttackNR();
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
            //Vector3.MoveTowards(transform.position, chasingTarget.position, chasingSpeed);

            transform.Translate(0.0f, 0.0f, chasingSpeed * Time.deltaTime);
            
        }
        else if ((transform.position - chasingTarget.position).magnitude < chasingTriggerDistance && trigerredAttack == false)
        {
            //Vector3.MoveTowards(transform.position, chasingTarget.position, chasingSpeed);

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

    private float sAttack1DT;
    
    void standardAttackNR()
    {
        sAttack1DT += Time.deltaTime;

        if (sAttack1DT >= 0.5f)
        {
            GameObject myBulletPrefab = Instantiate(sAttack1Bullet, sAttack1RightArm.position, Quaternion.identity) as GameObject;
            Rigidbody myBulletPrefabRigidBody = myBulletPrefab.GetComponent<Rigidbody>();
            myBulletPrefabRigidBody.AddForce(transform.forward * sAttack1BulletSpeed);

            GameObject myBulletPrefab2 = Instantiate(sAttack1Bullet, sAttack1LeftArm.position, Quaternion.identity) as GameObject;
            Rigidbody myBulletPrefabRigidBody2 = myBulletPrefab2.GetComponent<Rigidbody>();
            myBulletPrefabRigidBody2.AddForce(transform.forward * sAttack1BulletSpeed);

            sAttack1DT = 0;
            trigerredAttack = false;
        }
    }
    //---------------------------------------------------------------------------------
}
