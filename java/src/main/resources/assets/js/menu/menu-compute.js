/**
 * This file is responsible for all the logic in the compute tab of the menu.
 *
 * When things have to be calculated, this file will make sure this is send to the back-end.
 *
 * @author Jorrick Sleijster
 * @since  03/10/2018
 */


let __runningAlgorithm = false;

/**
 * Starts the computation of the evolution diagram for the bundles.
 */
function computeBundles(){
    let url = currentUrl + '/compute_bundles';
    computeInBackground(url);
}

function cutoffBundles(){
    let url = currentUrl + '/compute_bundle_cutoff';
    let dragBox = new ol.interaction.DragBox({});
    let rectangle = [0,0,0,0];

    dragBox.on('boxstart', (e) => {
        let lonlat = ol.proj.toLonLat(e.coordinate);
        let utm = convertLatLongtoUTM([lonlat[1],lonlat[0]]);
        rectangle[0] = utm[0];
        rectangle[1] = utm[1];
    });

    dragBox.on('boxend', (e) => {
        let lonlat = ol.proj.toLonLat(e.coordinate);
        let utm = convertLatLongtoUTM([lonlat[1],lonlat[0]]);
        rectangle[2] = utm[0];
        rectangle[3] = utm[1];
        map.removeInteraction(dragBox);

        if (rectangle[0] > rectangle[2]) {
            let rx = rectangle[0];
            rectangle[0] = rectangle[2];
            rectangle[2] = rx;
        }

        if (rectangle[1] > rectangle[3]) {
            let ry = rectangle[1];
            rectangle[1] = rectangle[3];
            rectangle[3] = ry;
        }

        computeInBackground(url + "?bounds=" + rectangle.join(","));
    });

    map.addInteraction(dragBox);
}

/**
 * Starts the computation of the RoadNetwork.
 */
function computeRoadNetwork(){
    let url = currentUrl + '/compute_network';
    computeInBackground(url);
}

/**
 * Starts the computation of the bundles and the road map in the back-end.
 */
function computeBundlesAndRoadMap() {
    let url = currentUrl + '/compute_bundles_and_network';
    computeInBackground(url);
}

/**
 * Starts an algorithm by loading the url
 * @param url
 */
function computeInBackground(url){
    if (!isAlgorithmRunning()){
        startAlgorithm();
        $.getJSON(url)
            .done(function (return_data) {
                if(return_data['error'] === false){
                    loadAllObjects();
                } else {
                    console.log(return_data);
                    console.log('Error computing bundles: ', return_data['errorMessage']);
                }
            })
            .fail(function (return_data){
                console.log('Error computing bundles: ', return_data);
            })
            .always(function (return_data){
                finishAlgorithm();
            });
    }
}

/**
 * Checks whether there is an algorithm running
 * @returns boolean whether the algorithm is running
 */
function isAlgorithmRunning(){
    return __runningAlgorithm === true;
}

/**
 * Starts the loader on top.
 */
function startAlgorithm(){
    __runningAlgorithm = true;
    NProgress.configure({
        trickle: false,
        showSpinner: false,
        minimum: 0.08
    });
    NProgress.start();
    updateAlgorithmProgress();
}

/**
 * Stops the loader on top.
 */
function finishAlgorithm(){
    NProgress.done();
    __runningAlgorithm = false;
}

/**
 * Updates the algorithm progress.
 */
function updateAlgorithmProgress(){
    if (isAlgorithmRunning()){
        $.getJSON(currentUrl + '/ping')
            .done(function (return_data) {
                let progress = return_data['algorithmProcess'];
                progress = (progress + 10)/110;
                NProgress.set(progress);
                // Update our status every 2 and a half second.
                setTimeout(updateAlgorithmProgress, 2500);
            });
    }
}