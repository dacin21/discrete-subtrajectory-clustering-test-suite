package mapconstruction.util;

import java.awt.geom.Point2D;

import static mapconstruction.GUI.datastorage.DataStorage.STORAGE;

/**
 * Coordinate conversion utility based on the JavaScript counterpart in {@code src/main/resources/js/deps/coordinate-converter.js}
 *
 * @author Jorren
 */
public class Coordinate {

    private final static double UTMScaleFactor = 0.9996d;
    private final static double sm_a = 6378137.0;
    private final static double sm_b = 6356752.314;

    public static Point2D UTMtoLatLon(Point2D coordinate) {
        boolean GGRS87 = STORAGE.getDatasetConfig().getSystem().equals("GGRS87");
        int xChange = GGRS87 ? 150 : 0;
        int yChange = GGRS87 ? 290 : 0;
        return UTMtoGeographic(new Point2D.Double(coordinate.getX() + xChange, coordinate.getY() + yChange));
    }

    public static Point2D UTMtoGeographic(Point2D coordinate) {
        double zone = STORAGE.getDatasetConfig().getZone();
        boolean isSouth = STORAGE.getDatasetConfig().getHemisphere().equals("S");

        Point2D utmxy = UTMXYtoLatLon(coordinate, zone, isSouth);

        return new Point2D.Double(Math.toDegrees(utmxy.getX()), Math.toDegrees(utmxy.getY()));
    }

    public static Point2D UTMXYtoLatLon(Point2D coordinate, double zone, boolean isSouth) {
        double cmeridian = UTMCentralMeridian(zone);

        Point2D adjusted = new Point2D.Double(
                (coordinate.getX() - 500000d) / UTMScaleFactor,
                (coordinate.getY() - (isSouth ? 10000000d : 0d)) / UTMScaleFactor);

        return mapXYtoLatLon(adjusted, cmeridian);
    }

    public static double UTMCentralMeridian(double zone) {
        return Math.toRadians(-183d + (zone * 6d));
    }

    public static Point2D mapXYtoLatLon(Point2D coordinate, double cmeridian) {
        double x = coordinate.getX(), y = coordinate.getY();
        double phif = footpointLatitude(y);
        double ep2 = (Math.pow(sm_a, 2d) - Math.pow(sm_b, 2d)) / Math.pow(sm_b, 2d);
        double cf = Math.cos(phif);
        double nuf2 = ep2 * Math.pow(cf, 2d);
        double Nf = Math.pow(sm_a, 2d) / (sm_b * Math.sqrt(1d + nuf2)), Nfpow = Nf;
        double tf = Math.tan(phif), tf2 = tf * tf, tf4 = tf2 * tf2;

        double x1frac = 1d / (Nfpow * cf);
        Nfpow *= Nf;
        double x2frac = tf / (2d * Nfpow);
        Nfpow *= Nf;
        double x3frac = 1d / (6d * Nfpow * cf);
        Nfpow *= Nf;
        double x4frac = tf / (24d * Nfpow);
        Nfpow *= Nf;
        double x5frac = 1d / (120d * Nfpow * cf);
        Nfpow *= Nf;
        double x6frac = tf / (720d * Nfpow);
        Nfpow *= Nf;
        double x7frac = 1d / (5040d * Nfpow * cf);
        Nfpow *= Nf;
        double x8frac = tf / (40320d * Nfpow);

        double x2poly = -1d - nuf2;
        double x3poly = -1d - 2d * tf2 - nuf2;
        double x4poly = 5d + 3d * tf2 + 6d * nuf2 - 6d * tf2 * nuf2 - 3d * (nuf2 * nuf2) - 9d * tf2 * (nuf2 * nuf2);
        double x5poly = 5d + 28d * tf2 + 24d * tf4 + 6d * nuf2 + 8d * tf2 * nuf2;
        double x6poly = -61d - 90d * tf2 - 45d * tf4 - 107d * nuf2 + 162d * tf2 * nuf2;
        double x7poly = -61d - 662d * tf2 - 1320d * tf4 - 720d * (tf4 * tf2);
        double x8poly = 1385d + 3633d * tf2 + 4095d * tf4 + 1575d * (tf4 * tf2);

        return new Point2D.Double(
            phif + x2frac * x2poly * Math.pow(x, 2)
                + x4frac * x4poly * Math.pow(x, 4)
                + x6frac * x6poly * Math.pow(x, 6)
                + x8frac * x8poly * Math.pow(x, 8),
            cmeridian + x1frac * x
                + x3frac * x3poly * Math.pow(x, 3)
                + x5frac * x5poly * Math.pow(x, 5)
                + x7frac * x7poly * Math.pow(x, 7)
        );
    }

    private static double footpointLatitude(double y) {
        double n = (sm_a - sm_b) / (sm_a + sm_b);
        double alpha = ((sm_a + sm_b) / 2d) * (1d + (Math.pow(n, 2d) / 4d) + (Math.pow(n, 4d) / 64d));
        double y_ = y / alpha;
        double beta_ = (3d * n / 2d) + (-27d * Math.pow(n, 3d) / 32d) + (269d * Math.pow(n, 5d) / 512d);
        double gamma_ = (21d * Math.pow(n, 2d) / 16d) + (-55d * Math.pow(n, 4d) / 32d);
        double delta_ = (151d * Math.pow(n, 3d) / 96d) + (-417d * Math.pow(n, 5d) / 128d);
        double epsilon_ = (1097d * Math.pow(n, 4d) / 512d);

        return y_ + (beta_ * Math.sin(2d * y_))
                + (gamma_ * Math.sin(4d * y_))
                + (delta_ * Math.sin(6d * y_))
                + (epsilon_ * Math.sin(8d * y_));
    }


}
