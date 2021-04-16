using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TurretBehaviour : MonoBehaviour
{
    private bool trigerredAttack;
    private float FireRate;

    private float Attack1Duration = 5.0f;
    private float reloadDuration = 2.0f;
    private bool reloading;

    // Start is called before the first frame update
    void Start()
    {
        trigerredAttack = false;
        reloading = false;
    }

    // Update is called once per frame
    void Update()
    {
        Attack1Duration -= Time.deltaTime;

        Aim();
       
        if(reloading != true)
        {
            if (trigerredAttack == true)
            {
                Shoot();
            }

            if (Attack1Duration < 0.0f)
            {
                reloadDuration = 2.0f;

                reloading = true;
            }
        }
        
        if(reloading == true)
        {
            reloadDuration -= Time.deltaTime;

            if (reloadDuration < 0.0f)
            {
                Attack1Duration = 5.0f;
                reloading = false;
            }
        }
        
        
    }

    //--------------------------------------------------------------------------------
    [Header("Aiming Settings")]
    public Transform aimingTarget;
    public float TriggerDistance = 8.0f;
    void Aim()
    {
        transform.LookAt(aimingTarget.transform);

        if ((transform.position - aimingTarget.position).magnitude < TriggerDistance && trigerredAttack == false)
        {

            trigerredAttack = true;

        }
    }
    //--------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------
    [Header("Shoot Settings")]
    public GameObject Bullet;
    public Transform Target;
    public Transform RightArm;
    public Transform LeftArm;
    public float fireRate;
    public float sAttack1BulletSpeed = 100;
    void Shoot()
    {
        FireRate += Time.deltaTime;

        if (FireRate >= 1.0f)
        {
            GameObject myBulletPrefab = Instantiate(Bullet, RightArm.position, Quaternion.identity) as GameObject;
            Rigidbody myBulletPrefabRigidBody = myBulletPrefab.GetComponent<Rigidbody>();
            myBulletPrefabRigidBody.AddForce(transform.forward * sAttack1BulletSpeed);

            GameObject myBulletPrefab2 = Instantiate(Bullet, LeftArm.position, Quaternion.identity) as GameObject;
            Rigidbody myBulletPrefabRigidBody2 = myBulletPrefab2.GetComponent<Rigidbody>();
            myBulletPrefabRigidBody2.AddForce(transform.forward * sAttack1BulletSpeed);

            FireRate = 0;
            trigerredAttack = false;
        }
 
    }
    //--------------------------------------------------------------------------------
}