using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Turret : MonoBehaviour
{


    public GameObject Target;

    private GameObject User;

    

    void Start()
    {

    

        
    }

    
    void Update()
    {


        Vector3 targetPos = new Vector3(Target.transform.position.x, Target.transform.position.y, Target.transform.position.z);

        


        this.transform.LookAt(targetPos);

        

    }
}
