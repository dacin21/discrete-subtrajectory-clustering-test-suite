package mapconstruction.web.config;

import com.fasterxml.jackson.annotation.JsonProperty;

import java.io.File;
import java.io.FileFilter;
import java.io.FilenameFilter;
import java.io.Serializable;
import java.util.LinkedHashMap;
import java.util.Map;

public class BundleConfig  implements Serializable {

    private String dataset;
    private String filemask;
    private boolean isWalking;

    @JsonProperty
    public String getDataset() {
        return dataset;
    }

    public void setDataset(String dataset) {
        this.dataset = dataset;
    }

    @JsonProperty
    public String getFilemask() {
        return filemask;
    }

    public void setFilemask(String filemask) {
        this.filemask = filemask;
    }

    public FilenameFilter filemask() {
        return filemask("");
    }

    public FilenameFilter filemask(String prefix) {
        return (dir, name) -> name.matches(prefix + filemask);
    }

    @JsonProperty
    public boolean getIsWalking() {
        return isWalking;
    }

    public void setIsWalking(boolean walking) {
        isWalking = walking;
    }

    public Map<String,Object> asMap() {
        Map<String,Object> config = new LinkedHashMap<>();
        config.put("dataset", dataset);
        config.put("filemask", filemask);
        config.put("isWalking", isWalking);
        return config;
    }

}
