import * as THREE from 'three';
import { STLLoader } from 'stlloader';
import { OrbitControls } from 'orbit';
import Stats from 'stats';
import { ViewHelper } from 'viewhelper';

THREE.Object3D.DefaultUp = new THREE.Vector3(0,0,1);

var stlFileName = localStorage.getItem("stlName");
document.title = "3D View: " + stlFileName;

const width = window.innerWidth, height = window.innerHeight;

const camera = new THREE.PerspectiveCamera(
    30,
    128 / 128,
    0.1,
    100000
)

const scene = new THREE.Scene();

const renderer = new THREE.WebGLRenderer({
    antialias: true,
    logarithmicDepthBuffer: true,
    alpha: true
})

renderer.setSize(window.innerWidth, window.innerHeight);
document.querySelector('#threejscanvas').appendChild( renderer.domElement );
renderer.shadowMap.enabled = true;
renderer.shadowMap.type = THREE.PCFSoftShadowMap;

const stats = new Stats();
document.body.appendChild(stats.dom);

document.getElementById("modeltitle").innerHTML += "<strong>" + "Loaded model file: " + "<br>" + "<i>" +  stlFileName + "</i>" + "</strong>";

const controls = new OrbitControls(camera, renderer.domElement);
controls.enableDamping = true;

const gridHelper = new THREE.GridHelper(10000, 500, 0x908e8f, 0x908e8f); // senzai mackerel
scene.add(gridHelper);

const gridSection = new THREE.GridHelper(10000, 100, 0xff6d8f, 0xff6d8f); // senzai hydrangea light
scene.add( gridSection );

const shadowPlaneGeom = new THREE.PlaneGeometry(100000, 100000);
const shadowPlaneMat = new THREE.ShadowMaterial();
shadowPlaneMat.opacity = 0.3;
const shadowPlane = new THREE.Mesh(shadowPlaneGeom, shadowPlaneMat);
shadowPlane.geometry.center();
shadowPlane.rotation.x = -Math.PI * 0.5;
shadowPlane.receiveShadow = true;
scene.add(shadowPlane);

const planeGeom = new THREE.PlaneGeometry(100000, 100000);
const planeMat = new THREE.MeshBasicMaterial({
    color: 0x2a262c, // senzai basalt
    side: THREE.FrontSide
});
const plane = new THREE.Mesh(planeGeom, planeMat);
plane.geometry.center();
plane.rotation.x = -Math.PI * 0.5;
scene.add(plane);

scene.fog = new THREE.Fog(0x2a262c, 20, 1500); // senzai basalt

const material = new THREE.MeshNormalMaterial();

var camOrigin = new THREE.Vector3(0,0,0);
var nearOrigin = 10;
var farOrigin = 100;

const loader = new STLLoader()
loader.load(
    stlFileName,
    function(geometry) {
        const mesh = new THREE.Mesh(geometry, material);
        mesh.geometry.center();
        mesh.rotation.x = -Math.PI * 0.5;

        var aabb = new THREE.Box3().setFromObject(mesh);
        var center = aabb.getCenter(new THREE.Vector3());
        var size = aabb.getSize(new THREE.Vector3());
        mesh.position.y = size.y/2;
        mesh.castShadow = true;
        mesh.traverse(function(child) {
          if (child.isMesh) {
            child.castShadow = true;
          }
        });
        mesh.receiveShadow = false;
        mesh.name = "mySTL";

        const light = new THREE.DirectionalLight(0xf5f5ef, 1); // senzai pebble
        light.position.set(size.x*0.1, size.y*2, -size.z*0.1);
        light.castShadow = true;
        light.shadow.radius = 2;
        light.shadow.mapSize.width = 4096;
        light.shadow.mapSize.height = 4096;
        light.shadow.camera.top = light.shadow.camera.right = 1000;
        light.shadow.camera.bottom = light.shadow.camera.left = -1000;
        light.shadow.camera.near = 0.1;
        light.shadow.camera.far = 400000000;

        scene.add(light);

        const btmLight = new THREE.DirectionalLight(0xf5f5ef, 1); // senzai pebble
        btmLight.position.set(size.x*0.1, -size.y*2, size.z*0.1);
        scene.add(btmLight);

        const leftLight = new THREE.DirectionalLight(0xf5f5ef, 1); // senzai pebble
        leftLight.position.set(-size.x, size.y, size.z*0.1);
        scene.add(leftLight);

        const rightLight = new THREE.DirectionalLight(0xf5f5ef, 1); // senzai pebble
        rightLight.position.set(size.x, size.y, size.z*0.1);
        scene.add(rightLight);

        const ambLight = new THREE.AmbientLight(0xf5f5ef); // senzai pebble
        scene.add(ambLight);

        camera.position.copy(mesh.position).add(new THREE.Vector3(center.x - size.x*2, center.y + size.y*2, (center .z + size.z) * 2.5));
        controls.target.copy(mesh.position);
        camera.lookAt(center);
        camera.aspect = window.innerWidth / window.innerHeight;
        camera.updateProjectionMatrix();

        camOrigin = camera.position.clone();

        if ((size.x > size.y ) && (size.x > size.z)) {
            scene.fog.near = size.x / 10;
            nearOrigin = scene.fog.near;
            scene.fog.far = size.x * 7;
            farOrigin = scene.fog.far;
        }
        else if ((size.y > size.x ) && (size.y > size.z)) {
            scene.fog.near = size.y / 10;
            nearOrigin = scene.fog.near;
            scene.fog.far = size.y * 7;
            farOrigin = scene.fog.far;
        }
        else {
            scene.fog.near = size.z / 10;
            nearOrigin = scene.fog.near;
            scene.fog.far = size.z * 7;
            farOrigin = scene.fog.far;
        }

        renderer.setSize(window.innerWidth, window.innerHeight);

        scene.add(mesh);
    },
    (xhr) => {
        console.log((xhr.loaded / xhr.total) * 100 + '% loaded');
    },
    (error) => {
        console.log(error);
    }
)

function matToggle() {
  var checkBox = document.getElementById("my-toggle");
  var object = scene.getObjectByName("mySTL");

  if (checkBox.checked == true){
    object.material = new THREE.MeshLambertMaterial({color: 0xf6d251}); // senzai yuzu dark
    object.material.metalness = 0;
    object.material.fog = false;
  }
  else {
    object.material = new THREE.MeshNormalMaterial();
  }
}

window.matToggle= matToggle;

renderer.autoClear = false;

var helper = new ViewHelper(camera, renderer.domElement);

window.addEventListener('resize', onWindowResize, false);

function onWindowResize() {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
    render();
}

function animate() {
    requestAnimationFrame(animate);
    controls.update();

    var scaleVector = new THREE.Vector3();
    var scaleFactor = scaleVector.subVectors(plane.position, camOrigin).length();
    var zoomScale = scaleVector.subVectors(plane.position, camera.position).length() / scaleFactor * 100;

    scene.fog.near = nearOrigin + zoomScale;
    scene.fog.far = farOrigin + zoomScale * 2;

    render();
}

function render() {
    renderer.clear();
    renderer.render(scene, camera);
    helper.render(renderer);
    stats.update();
}

animate();